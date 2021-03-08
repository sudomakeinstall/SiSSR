#ifndef itk_LoopSubdivisionSurfaceMatrices_hxx
#define itk_LoopSubdivisionSurfaceMatrices_hxx

// STD
#include  <cmath>

// VNL
#include <vnl/vnl_complexify.h>
#include <vnl/vnl_complex_ops.hxx>
#include <vnl/vnl_power.h>
#include <vnl/vnl_index_sort.h>
#include <vnl/vnl_math.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>
#include <vnl/algo/vnl_real_eigensystem.h>

// ITK
#include "itkLoopSubdivisionSurfaceMatrices.h"
#include "itkMacro.h"

// Custom
#include "dvMath.h"
#include "dv_sylvester.h"

namespace itk
{

/////////////////
// Constructor //
/////////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::LoopSubdivisionSurfaceMatrices() :
  m_BSplineM(this->CalculateBSplineThinPlateEnergyMatrix()),
  m_BSplineM_block(this->CalculateBSplineThinPlateEnergyMatrixBlock()),
  m_BSplineM_root(this->CalculateBSplineThinPlateEnergyMatrixRoot()),
  m_BSplineM_root_block(this->CalculateBSplineThinPlateEnergyMatrixRootBlock()),
  m_BezierM(this->CalculateBezierThinPlateEnergyMatrix()),
  m_BezierM_block(this->CalculateBezierThinPlateEnergyMatrixBlock()),
  m_BezierM_root(this->CalculateBezierThinPlateEnergyMatrixRoot()),
  m_BezierM_root_block(this->CalculateBezierThinPlateEnergyMatrixRootBlock())
{

  for (unsigned int n = MinN; n <= MaxN; ++n)
    {
    this->m_S_map.emplace(n, this->CalculateS(n));
    this->m_A_map.emplace(n, this->CalculateA(n));
    this->m_B_map.emplace(n, this->CalculateB(n));

    typename Self::TPickers p;
    p.at(0) = this->CalculateP(n,0);
    p.at(1) = this->CalculateP(n,1);
    p.at(2) = this->CalculateP(n,2);
    this->m_P_map.emplace(n, p);

    this->m_S_Eigenvalues_map.emplace(n, this->CalculateSEigenvalues(n));
    this->m_A_Eigenvalues_map.emplace(n, this->CalculateAEigenvalues(n));

    this->m_S_Eigenvectors_map.emplace(n, this->CalculateSEigenvectors(n));
    this->m_A_Eigenvectors_map.emplace(n, this->CalculateAEigenvectors(n));

    this->m_Sorted_Eigensystem_map.emplace(n, this->CalculateSortedEigensystem(n));

    this->m_BSplineToBezier_map.emplace(n, this->CalculateBSplineToBezierMatrix(n));
    }
}

///////////////
// Utilities //
///////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
TReal
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateAlpha(const unsigned int &N)
{
  return 5./8 - pow((3.+2*cos(2.*M_PI/N)),2)/64;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
TReal
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::f(const unsigned int &k, const unsigned int &N)
{
  return 3./8. + 2./8.*cos(2*M_PI*k/N);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
int
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateNumberOfRequiredSubdivisions(const TParameters &p)
{
  
  int n = floor(1-log2(p.first+p.second));
  itkAssertOrThrowMacro(0 < n, "Invalid number of required subdivisions: " + std::to_string(n));
  return n;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
int
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateChildIndex(const TParameters &p)
{

  const int n = CalculateNumberOfRequiredSubdivisions(p);
  const TReal LowerBound = 0;
  const TReal MiddleBound = pow(2,-n);
  const TReal UpperBound = pow(2,1-n);

  if (   dv::IsBetween(p.first,MiddleBound,UpperBound)
      && dv::IsBetween(p.second,LowerBound,UpperBound-p.first))
    {
    return 0;
    }
  else if (dv::IsBetween(p.first,LowerBound,MiddleBound))
    {
    if (dv::IsBetween(p.second,MiddleBound-p.first,MiddleBound))
      {
      return 1;
      }
    else if (dv::IsBetween(p.second,MiddleBound,UpperBound-p.first))
      {
      return 2;
      }
    }
  itkAssertOrThrowMacro(false, "Invalid child index calculated.");
  return -1;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
TReal
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::BarycentricBSplineWeightScale(const unsigned int &order,
                         const unsigned int &k,
                         const unsigned int &n)
{
  return std::pow(2.0, order * n) * std::pow( ( k == 1 ? -1 : 1 ), order);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
bool
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::VerifyParameters(const TParameters &p)
{
  bool allGood = true;
  if (0 > p.first || 1 < p.first)
    {
    std::cerr << "The first parameter is not in the range [0,1]: "
              << p.first
              << std::endl;
    allGood = false;
    }
  if (0 > p.second || 1 < p.second)
    {
    std::cerr << "The second parameter is not in the range [0,1]: "
              << p.second
              << std::endl;
    allGood = false;
    }
  if (1 < (p.first+p.second) || 0 > (p.first+p.second))
    {
    std::cerr << "The sum of the two parameters is not in the range [0,1]: "
              << p.first + p.second
              << std::endl;
    allGood = false;
    }
  return allGood;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
typename LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>::TParameters
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::TransformParametersToPatch(const TParameters &p)
{
  const int n = CalculateNumberOfRequiredSubdivisions(p);
  const int k = CalculateChildIndex(p);
  const TReal coef = pow(2,n);
  TReal first, second;

  switch (k)
    {
    case 0:
      first = coef*p.first-1;
      second = coef*p.second;
      break;
    case 1:
      first = 1-coef*p.first;
      second = 1-coef*p.second;
      break;
    case 2:
      first = coef*p.first;
      second = coef*p.second-1;
      break;
    default:
      itkAssertOrThrowMacro(false,"The child index must be 0, 1, or 2"); 
      break; 
    }

  return std::make_pair(first, second);

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateP(const unsigned int &N, const unsigned int &k)
{
  const unsigned int M = N+12;
  vnl_matrix<TReal> s(12,M);
  s.fill(TReal(0));
  switch (k) {
    case 0:
      s( 0,  1) = 1;
      s( 1,N+2) = 1;
      s( 2,N+3) = 1;
      s( 3,  2) = 1;
      s( 4,  0) = 1;
      s( 5,  N) = 1;
      s( 6,N+1) = 1;
      s( 7,N+6) = 1;
      s( 8,N+7) = 1;
      s( 9,N+8) = 1;
      s(10,N+9) = 1;
      s(11,N+4) = 1;
      break;
    case 1:
      s( 0,N+1) = 1;
      s( 1,  N) = 1;
      s( 2,N+4) = 1;
      s( 3,N+9) = 1;
      s( 4,N+6) = 1;
      s( 5,N+2) = 1;
      s( 6,  1) = 1;
      s( 7,  0) = 1;
      s( 8,N-1) = 1;
      s( 9,N+5) = 1;
      s(10,  2) = 1;
      s(11,N+3) = 1;
      break;
    case 2:
      s( 0,   N) = 1;
      s( 1, N+1) = 1;
      s( 2,   1) = 1;
      s( 3,   0) = 1;
      s( 4, N-1) = 1;
      s( 5, N+5) = 1;
      s( 6, N+4) = 1;
      s( 7, N+9) = 1;
      s( 8, N+6) = 1;
      s( 9, N+2) = 1;
      s(10,N+10) = 1;
      s(11,N+11) = 1;
      break;
    default:
      std::cerr << "The index supplied was not valid." << std::endl;
      break;
  }
  return s;
}

/////////////////////////////////
// Barycentric BSpline Weights //
/////////////////////////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector_fixed<TReal,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBarycentricBSplineWeights(const TParameters &p)
{

  const TReal s = p.first;
  const TReal t = p.second;

  const TReal s1 =     s;
  const TReal s2 = pow(s,2);
  const TReal s3 = pow(s,3);
  const TReal s4 = pow(s,4);

  const TReal t1 =     t;
  const TReal t2 = pow(t,2);
  const TReal t3 = pow(t,3);
  const TReal t4 = pow(t,4);

  vnl_vector_fixed<TReal,12> wts; 
  wts[ 0] = -s4-2*s3*t1+8*s3+12*s2*t1-12*s2-2*s1*t3+12*s1*t2-12*s1*t1-t4+8*t3-12*t2+6;
  wts[ 1] = -s4-2*s3*t1-4*s3-6*s2*t1+6*s2+4*s1*t3-12*s1*t2+6*s1*t1+4*s1+2*t4-4*t3+2*t1+1;
  wts[ 2] = 2*s4+4*s3*t1-4*s3-2*s1*t3+6*s1*t2-6*s1*t1+2*s1-t4+2*t3-2*t1+1;
  wts[ 3] = -s4-2*s3*t1+2*s3+2*s1*t3-6*s1*t2+6*s1*t1-2*s1+t4-4*t3+6*t2-4*t1+1;
  wts[ 4] = s4+2*s3*t1-4*s3-6*s2*t1+6*s2-2*s1*t3+6*s1*t1-4*s1-t4+2*t3-2*t1+1;
  wts[ 5] = -s4-2*s3*t1+2*s3+6*s2*t1+4*s1*t3-6*s1*t1-2*s1+2*t4-4*t3+2*t1+1;
  wts[ 6] = 2*s4+4*s3*t1-4*s3-12*s2*t1-2*s1*t3-6*s1*t2+6*s1*t1+2*s1-t4-4*t3+6*t2+4*t1+1;
  wts[ 7] = -s4-2*s3*t1+2*s3+6*s2*t1-2*s1*t3+6*s1*t2-t4+2*t3;
  wts[ 8] = s4+2*s3*t1;
  wts[ 9] = -s4-2*s3*t1+2*s3;
  wts[10] = 2*s1*t3+t4;
  wts[11] = -2*s1*t3-t4+2*t3;
  
  wts /= 12.; 

  return wts;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector_fixed<TReal,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBarycentricBSplineWeightsDR(const TParameters &p)
{

  const TReal s = p.first;
  const TReal t = p.second;
  const TReal r = 1 - s - t;
  itkAssertOrThrowMacro(std::fabs((r + s + t) - 1) < 10e-5, "");

  const TReal s1 =     s;
  const TReal s2 = pow(s,2);
  const TReal s3 = pow(s,3);

  const TReal t1 =     t;
  const TReal t2 = pow(t,2);
  const TReal t3 = pow(t,3);

  const TReal r1 =     r;
  const TReal r2 = pow(r,2);
  const TReal r3 = pow(r,3);

  vnl_vector_fixed<TReal,12> wts; 
  wts[ 0] = 2*r3+6*r2*s1+6*r2*t1+4*r1*s2+10*r1*s1*t1+4*r1*t2+2*s3/3+3*s2*t1+3*s1*t2+2*t3/3;
  wts[ 1] = r3/3+2*r2*s1+3*r2*t1/2+4*r1*s2+6*r1*s1*t1+2*r1*t2+2*s3+5*s2*t1+3*s1*t2+t3/2;
  wts[ 2] = r3/3+3*r2*s1/2+r2*t1/2+2*r1*s2+r1*s1*t1+s3/2+s2*t1/2;
  wts[ 3] = r3/3+r2*s1/2;
  wts[ 4] = r3/3+r2*t1/2;
  wts[ 5] = r3/3+r2*s1/2+3*r2*t1/2+r1*s1*t1+2*r1*t2+s1*t2/2+t3/2;
  wts[ 6] = r3/3+3*r2*s1/2+2*r2*t1+2*r1*s2+6*r1*s1*t1+4*r1*t2+s3/2+3*s2*t1+5*s1*t2+2*t3;
  wts[ 7] = s3/6+s2*t1/2+s1*t2/2+t3/6;
  wts[ 8] = 0;
  wts[ 9] = s3/6;
  wts[10] = 0;
  wts[11] = t3/6;

  const unsigned int order = 1;
  const auto k = Self::CalculateChildIndex(p);
  const auto n = Self::CalculateNumberOfRequiredSubdivisions(p);
  return wts * Self::BarycentricBSplineWeightScale(order, k, n);

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector_fixed<TReal,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBarycentricBSplineWeightsDS(const TParameters &p)
{

  const TReal s = p.first;
  const TReal t = p.second;
  const TReal r = 1 - s - t;
  itkAssertOrThrowMacro(std::fabs((r + s + t) - 1) < 10e-5, "");

  const TReal s1 =     s;
  const TReal s2 = pow(s,2);
  const TReal s3 = pow(s,3);

  const TReal t1 =     t;
  const TReal t2 = pow(t,2);
  const TReal t3 = pow(t,3);

  const TReal r1 =     r;
  const TReal r2 = pow(r,2);
  const TReal r3 = pow(r,3);

  vnl_vector_fixed<TReal,12> wts; 
  wts[ 0] = 2*r3+4*r2*s1+5*r2*t1+2*r1*s2+6*r1*s1*t1+3*r1*t2+s3/3+3*s2*t1/2+2*s1*t2+t3/2;
  wts[ 1] = 2*r3/3+4*r2*s1+3*r2*t1+6*r1*s2+10*r1*s1*t1+3*r1*t2+2*s3+6*s2*t1+4*s1*t2+2*t3/3;
  wts[ 2] = r3/2+2*r2*s1+r2*t1/2+3*r1*s2/2+r1*s1*t1+s3/3+s2*t1/2;
  wts[ 3] = r3/6;
  wts[ 4] = 0;
  wts[ 5] = r3/6+r2*t1/2+r1*t2/2+t3/6;
  wts[ 6] = r3/2+2*r2*s1+3*r2*t1+3*r1*s2/2+6*r1*s1*t1+5*r1*t2+s3/3+2*s2*t1+4*s1*t2+2*t3;
  wts[ 7] = r1*s2/2+r1*s1*t1+r1*t2/2+s3/3+3*s2*t1/2+2*s1*t2+t3/2;
  wts[ 8] = s3/3+s2*t1/2;
  wts[ 9] = r1*s2/2+s3/3;
  wts[10] = t3/6;
  wts[11] = 0;

  const unsigned int order = 1;
  const auto k = Self::CalculateChildIndex(p);
  const auto n = Self::CalculateNumberOfRequiredSubdivisions(p);
  return wts * Self::BarycentricBSplineWeightScale(order, k, n);

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector_fixed<TReal,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBarycentricBSplineWeightsDT(const TParameters &p)
{

  const TReal s = p.first;
  const TReal t = p.second;
  const TReal r = 1 - s - t;
  itkAssertOrThrowMacro(std::fabs((r + s + t) - 1) < 10e-5, "");

  const TReal s1 =     s;
  const TReal s2 = pow(s,2);
  const TReal s3 = pow(s,3);

  const TReal t1 =     t;
  const TReal t2 = pow(t,2);
  const TReal t3 = pow(t,3);

  const TReal r1 =     r;
  const TReal r2 = pow(r,2);
  const TReal r3 = pow(r,3);

  vnl_vector_fixed<TReal,12> wts; 
  wts[ 0] = 2*r3+5*r2*s1+4*r2*t1+3*r1*s2+6*r1*s1*t1+2*r1*t2+s3/2+2*s2*t1+3*s1*t2/2+t3/3;
  wts[ 1] = r3/2+3*r2*s1+2*r2*t1+5*r1*s2+6*r1*s1*t1+3*r1*t2/2+2*s3+4*s2*t1+2*s1*t2+t3/3;
  wts[ 2] = r3/6+r2*s1/2+r1*s2/2+s3/6;
  wts[ 3] = 0;
  wts[ 4] = r3/6;
  wts[ 5] = r3/2+r2*s1/2+2*r2*t1+r1*s1*t1+3*r1*t2/2+s1*t2/2+t3/3;
  wts[ 6] = 2*r3/3+3*r2*s1+4*r2*t1+3*r1*s2+10*r1*s1*t1+6*r1*t2+2*s3/3+4*s2*t1+6*s1*t2+2*t3;
  wts[ 7] = r1*s2/2+r1*s1*t1+r1*t2/2+s3/2+2*s2*t1+3*s1*t2/2+t3/3;
  wts[ 8] = s3/6;
  wts[ 9] = 0;
  wts[10] = s1*t2/2+t3/3;
  wts[11] = r1*t2/2+t3/3;

  const unsigned int order = 1;
  const auto k = Self::CalculateChildIndex(p);
  const auto n = Self::CalculateNumberOfRequiredSubdivisions(p);
  return wts * Self::BarycentricBSplineWeightScale(order, k, n);

}

//////////////////////////
// Subdivision Matrices //
//////////////////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS(const unsigned int &N)
{

  const TReal alpha = Self::CalculateAlpha(N);
  const TReal a = 1-alpha;
  const TReal b = alpha/N;
  const TReal c = 3./8;
  const TReal d = 1./8;

  vnl_matrix<TReal> s(N+1,N+1,0);
  s.set_column(0,c);
  s.set_row(0,b);
  s.fill_diagonal(c);
  s(0,0) = a;
  for (unsigned int i = 0; i < N; ++i)
    {
    s(i+1,vnl_math::remainder_floored(int(i+1),int(N))+1) = d;
    s(i+1,vnl_math::remainder_floored(int(i-1),int(N))+1) = d;
    }

  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS11(const unsigned int &N)
{

  vnl_matrix<TReal> s(5,N+1,0);

  s(0,0) = 2;
  s(0,1) = 6;
  s(0,N) = 6;

  s(1,0) = 1;
  s(1,1) = 10;
  s(1,2) = 1;
  s(1,N) = 1;

  s(2,0) = 2;
  s(2,1) = 6;
  s(2,2) = 6;

  s(3,0) = 1;
  s(3,1) = 1;
  s(3,N-1) = 1;
  s(3,N) = 10;

  s(4,0) = 2;
  s(4,N-1) = 6;
  s(4,N) = 6;

  return s/TReal(16);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,5,5>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS12()
{

  // Casting is required to prevent an ambiguous call error
  vnl_matrix_fixed<TReal,5,5> s(TReal(0));
  s(0,0) = 2;
  s(1,0) = s(1,1) = s(1,2) = 1;
  s(2,2) = 2;
  s(3,0) = s(3,3) = s(3,4) = 1;
  s(4,4) = 2;
  return s/TReal(16);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateA(const unsigned int &N)
{
  const unsigned int K = N + 6;
  vnl_matrix<TReal> s(K,K);
  s.fill(TReal(0));
  s.update(Self::CalculateS(N).as_ref());
  s.update(Self::CalculateS11(N).as_ref(),N+1,0);
  s.update(Self::CalculateS12().as_ref(),N+1,N+1);
  return s;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS21(const unsigned int &N)
{
  vnl_matrix<TReal> s(6,N+1);
  s.fill(TReal(0));
  s(0,1) = 3;
  s(0,N) = 1;
  s(1,1) = 3;
  s(2,1) = 3;
  s(2,2) = 1;
  s(3,1) = 1;
  s(3,N) = 3;
  s(4,N) = 3;
  s(5,N-1) = 1;
  s(5,N) = 3;
  return s/TReal(8);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,6,5>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS22()
{
  vnl_matrix_fixed<TReal,6,5> s;
  s.fill(TReal(0));
  s(0,0) = 3;
  s(0,1) = 1;
  s(1,0) = 1;
  s(1,1) = 3;
  s(1,2) = 1;
  s(2,1) = 1;
  s(2,2) = 3;
  s(3,0) = 3;
  s(3,3) = 1;
  s(4,0) = 1;
  s(4,3) = 3;
  s(4,4) = 1;
  s(5,3) = 1;
  s(5,4) = 3;
  return s/TReal(8);
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateB(const unsigned int &N)
{

  const unsigned int K = N + 6;
  const unsigned int M = K + 6;
  vnl_matrix<TReal> s(M,K);
  s.fill(TReal(0));
  s.update(Self::CalculateA(N).as_ref());
  s.update(Self::CalculateS21(N).as_ref(),K,0);
  s.update(Self::CalculateS22().as_ref(),K,N+1);
  return s;

}

////////////
// Bezier //
////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,15,15>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBezierThinPlateEnergyMatrix()
{

  const TReal data[255] =
{ 30135 ,  6480  ,   87  ,  3444 , -34020,  -2160 ,  4752  ,  1116 ,  -972 ,   720  ,  4176  ,  1434 , -14796 ,  4608  ,  -5004 ,
   6480 , 559872 ,  6480 , 15552 , -25920, -559872, -559872, -25920, 15552 , 165888 , 165888 , 18144 , -36288 , 290304 , -36288 , 
    87  ,  6480  , 30135 ,  -972 ,  1116 ,  4752  ,  -2160 , -34020,  3444 ,  4176  ,   720  ,  1434 ,  -5004 ,  4608  , -14796 , 
   3444 ,  15552 ,  -972 , 71664 , -62928,  12096 ,  -1728 , -4176 ,  2160 ,  16128 ,  9216  , 22200 , -48240 ,  5760  , -40176 , 
  -34020, -25920 ,  1116 , -62928, 122400,  17280 ,  3456  ,  6048 , -4176 , -64800 ,  -2592 , -15768,  45792 , -12096 ,  26208 , 
  -2160 , -559872,  4752 , 12096 , 17280 , 1036800, 207360 ,  3456 , -1728 , -404352,  10368 ,  7776 ,  10368 , -393984,  51840 , 
   4752 , -559872, -2160 , -1728 ,  3456 , 207360 , 1036800, 17280 , 12096 ,  10368 , -404352,  7776 ,  51840 , -393984,  10368 , 
   1116 , -25920 , -34020, -4176 ,  6048 ,  3456  ,  17280 , 122400, -62928,  -2592 , -64800 , -15768,  26208 , -12096 ,  45792 , 
   -972 ,  15552 ,  3444 ,  2160 , -4176 ,  -1728 ,  12096 , -62928, 71664 ,  9216  ,  16128 , 22200 , -40176 ,  5760  , -48240 , 
   720  , 165888 ,  4176 , 16128 , -64800, -404352,  10368 , -2592 ,  9216 , 361152 ,  15552 , 38880 , -111456, -17280 , -21600 , 
   4176 , 165888 ,  720  ,  9216 , -2592 ,  10368 , -404352, -64800, 16128 ,  15552 , 361152 , 38880 , -21600 , -17280 , -111456,
   1434 ,  18144 ,  1434 , 22200 , -15768,  7776  ,  7776  , -15768, 22200 ,  38880 ,  38880 , 81468 , -87048 , -34560 , -87048 , 
  -14796, -36288 , -5004 , -48240, 45792 ,  10368 ,  51840 , 26208 , -40176, -111456, -21600 , -87048, 262944 , -77760 ,  45216 , 
   4608 , 290304 ,  4608 ,  5760 , -12096, -393984, -393984, -12096,  5760 , -17280 , -17280 , -34560, -77760 , 725760 , -77760 , 
  -5004 , -36288 , -14796, -40176, 26208 ,  51840 ,  10368 , 45792 , -48240, -21600 , -111456, -87048,  45216 , -77760 , 262944 };

  vnl_matrix_fixed<TReal,15,15> s(data);
  s /= TReal(2560);

  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,45,45>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBezierThinPlateEnergyMatrixBlock()
{

  vnl_matrix_fixed<TReal,45,45> s(TReal(0));
  const auto M = Self::CalculateBezierThinPlateEnergyMatrix();
  s.update(M.as_ref(), 0, 0);
  s.update(M.as_ref(),15,15);
  s.update(M.as_ref(),30,30);

  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,15,15>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBezierThinPlateEnergyMatrixRoot()
{

//  const auto M = Self::CalculateBezierThinPlateEnergyMatrix();
//  const auto M_root = vnl_power(M.as_matrix(), 0.5);
//  return M_root;

  const auto M = Self::CalculateBezierThinPlateEnergyMatrix();
  const auto sys = vnl_symmetric_eigensystem<TReal>(M.as_ref());
  const auto vec = sys.V;
  // sys.D returns a vector, but we neex a matrix.  Do the necessary conversion.
  auto val = vnl_matrix<TReal>(sys.D.as_matrix());
  // The matrix *should* be all real and positive,
  // but small negative numbers may be introduced through numerical error.
  for (auto it = val.begin(); it != val.end(); ++it)
    {
    if (10e-6 > std::fabs(*it)) *it = 0;
    // If any *large* negative values get through, something has gone wrong.
    itkAssertOrThrowMacro(0 <= *it,"Eigenvalues should not be negative.");
    }
  // Since there are no negative eigenvalues, we avoid complex numbers.
  const auto val_root = val.apply(std::sqrt);
  const auto M_sqrt = vec.as_ref() * val_root.as_ref() * vnl_matrix_inverse<TReal>(vec).as_matrix();
  vnl_matrix_fixed<TReal,15,15> M_sqrt_fixed(M_sqrt);
  return M_sqrt_fixed;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,45,45>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBezierThinPlateEnergyMatrixRootBlock()
{

  vnl_matrix_fixed<TReal,45,45> s(TReal(0));
  const auto M = Self::CalculateBezierThinPlateEnergyMatrixRoot();
  s.update(M.as_ref(), 0, 0);
  s.update(M.as_ref(),15,15);
  s.update(M.as_ref(),30,30);

  return s;

}

/////////////
// BSpline //
/////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,12,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBSplineThinPlateEnergyMatrix()
{
  const TReal data[144] = {+114,-  6,- 33,-  9,-  9,- 33,-  6,+  6,+  3,- 15,+  3,- 15,
                           -  6,+114,- 33,- 15,+  3,+  6,-  6,- 33,-  9,-  9,- 15,+  3,
                           - 33,- 33,+ 42,+  3,-  6,+  3,+  6,+  3,-  6,+  3,+  9,+  9,
                           -  9,- 15,+  3,+ 10,+  2,-  6,+  3,+  9,+  1,-  1,+  2,+  1,
                           -  9,+  3,-  6,+  2,+ 10,+  3,- 15,+  9,+  2,+  1,+  1,-  1,
                           - 33,+  6,+  3,-  6,+  3,+ 42,- 33,+  3,+  9,+  9,-  6,+  3,
                           -  6,-  6,+  6,+  3,- 15,- 33,+114,- 33,- 15,+  3,-  9,-  9,
                           +  6,- 33,+  3,+  9,+  9,+  3,- 33,+ 42,+  3,-  6,+  3,-  6,
                           +  3,-  9,-  6,+  1,+  2,+  9,- 15,+  3,+ 10,+  2,-  1,+  1,
                           - 15,-  9,+  3,-  1,+  1,+  9,+  3,-  6,+  2,+ 10,+  1,+  2,
                           +  3,- 15,+  9,+  2,+  1,-  6,-  9,+  3,-  1,+  1,+ 10,+  2,
                           - 15,+  3,+  9,+  1,-  1,+  3,-  9,-  6,+  1,+  2,+  2,+ 10};

  vnl_matrix_fixed<TReal,12,12> s(data);
  s /= TReal(162);
  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,36,36>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBSplineThinPlateEnergyMatrixBlock()
{

  vnl_matrix_fixed<TReal,36,36> s(TReal(0));
  const auto M = Self::CalculateBSplineThinPlateEnergyMatrix();
  s.update(M.as_ref(), 0, 0);
  s.update(M.as_ref(),12,12);
  s.update(M.as_ref(),24,24);

  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,12,12>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBSplineThinPlateEnergyMatrixRoot()
{

  const auto M = Self::CalculateBSplineThinPlateEnergyMatrix();
  const auto sys = vnl_symmetric_eigensystem<TReal>(M.as_ref());
  const auto vec = sys.V;
  // sys.D returns a vector, but we neex a matrix.  Do the necessary conversion.
  auto val = vnl_matrix<TReal>(sys.D.as_matrix());
  // The matrix *should* be all real and positive,
  // but small negative numbers may be introduced through numerical error.
  for (auto it = val.begin(); it != val.end(); ++it)
    {
    if (10e-6 > std::fabs(*it)) *it = 0;
    // If any *large* negative values get through, something has gone wrong.
    itkAssertOrThrowMacro(0 <= *it,"Eigenvalues should not be negative.");
    }
  // Since there are no negative eigenvalues, we avoid complex numbers.
  const auto val_root = val.apply(std::sqrt);
  const auto M_sqrt = vec.as_ref() * val_root.as_ref() * vnl_matrix_inverse<TReal>(vec).as_matrix();
  vnl_matrix_fixed<TReal,12,12> M_sqrt_fixed(M_sqrt);
  return M_sqrt_fixed;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,36,36>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBSplineThinPlateEnergyMatrixRootBlock()
{

  vnl_matrix_fixed<TReal,36,36> s(TReal(0));
  const auto M_sqrt = Self::CalculateBSplineThinPlateEnergyMatrixRoot();
  s.update(M_sqrt.as_ref(), 0, 0);
  s.update(M_sqrt.as_ref(),12,12);
  s.update(M_sqrt.as_ref(),24,24);

  return s;

}

/////////////////
// Eigenvalues //
/////////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateAEigenvalues(const unsigned int &N)
{
  const unsigned int K = N + 6;
  vnl_vector<TReal> s(K,TReal(0));
  const auto Sigma = Self::CalculateSEigenvalues(N);
  const auto Delta = Self::CalculateS12Eigenvalues();
  s.update(Sigma.as_ref());
  s.update(Delta.as_ref(),N+1);
  return s;
}

// In Stam's paper, this is Sigma
template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateSEigenvalues(const unsigned int &N)
{
  vnl_vector<TReal> s(N+1,TReal(0));
  s.put(0,1); // u1
  s.put(1,5./8. - Self::CalculateAlpha(N)); // u2
  for (unsigned int i = 3; i <= N+1; ++i)
    s.put(i-1,f(i-2,N));
  return s;
}

// In Stam's paper, this is Delta.
template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_vector_fixed<TReal,5>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS12Eigenvalues()
{
  const TReal data[5] = {2, 2, 2, 1, 1};
  vnl_vector_fixed<TReal,5> s(data);
  s /= 16.;
  return s;
}

//////////////////
// EIGENVECTORS //
//////////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix< typename LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>::TComplex >
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateComplexSEigenvectors(const unsigned int &N)
{
  vnl_matrix<TComplex> s(N+1,N+1);
  // First column is all 1s.
  // Second column is all 1s except for the first entry.
  s.fill(TComplex(1.0,0.0));
  s.set_row(0,TComplex(0.0,0.0));
  s.put(0,0,TComplex(1.0,0.0));
  s.put(0,1,TComplex(-8./3.*Self::CalculateAlpha(N),0.0));
  vnl_matrix<TReal> col_indices(N-1,1); // Column vector
  vnl_matrix<TReal> row_indices(1,N-1); // Row vector
  for (unsigned int i = 0; i < N-1; ++i)
    {
    col_indices.put(i,0,i+1);
    row_indices.put(0,i,i+1);
    }
  auto indices = vnl_complexify(col_indices*row_indices);
  for (auto it = indices.begin(); it != indices.end(); ++it)
    {
    *it = std::exp(TComplex(TReal(0),2.*M_PI*(it->real())/TReal(N)));
    }
  s.update(indices,2,2);


  return s;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateSEigenvectors(const unsigned int &N)
{

  auto c = Self::CalculateComplexSEigenvectors(N);

  // Corresponding vectors (e.g., 3 and 7 for N=6)
  // have the same real component, and imaginary components
  // of opposite sign.  Therefore, real vectors can be recovered
  // as r1 = (u3 + u7)/2 and r2 = (u3 - u7) / (2i).

  if (1 == N % 2) // odd
    {
    for (unsigned int u = 3; u < (N+1-2)/2+3; ++u)
      {
      const auto l_c = c.get_column(u-1);
      const auto r_c = c.get_column(N-(u-3));
      const auto l_r = (l_c+r_c)/TComplex(2.0,0.0);
      const auto r_r = (l_c-r_c)/TComplex(0.0,2.0);
      c.set_column(u-1,l_r);
      c.set_column(N-(u-3),r_r);
      }
    }
  else // even
    {
    for (unsigned int u = 3; u < (N+1-2-1)/2+3; ++u)
      {
      const auto l_c = c.get_column(u-1);
      const auto r_c = c.get_column(N-(u-3));
      const auto l_r = (l_c+r_c)/TComplex(2.0,0.0);
      const auto r_r = (l_c-r_c)/TComplex(0.0,2.0);
      c.set_column(u-1,l_r);
      c.set_column(N-(u-3),r_r);
      }
    }

  itkAssertOrThrowMacro(vnl_imag(c).array_two_norm() < 10e-5, "");

  return vnl_real(c);

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS11Eigenvectors(const unsigned int& N)
{

  using TSylvester = dv_sylvester<TReal>;

  const auto sigma = Self::CalculateSEigenvalues(N);
  const auto s12   = Self::CalculateS12();
  const auto s11   = Self::CalculateS11(N);
  const auto u0    = Self::CalculateSEigenvectors(N);

  const vnl_diag_matrix<TReal> A(sigma);
  const auto B = (TReal(-1) * s12).transpose();

  vnl_matrix<TReal> s11_copy(s11.data_block(), s11.rows(), s11.cols());
  vnl_matrix<TReal> u0_copy(u0.data_block(), u0.rows(), u0.cols());
  const auto Q = (s11_copy * u0_copy).transpose();

  const TSylvester sylvester(A.as_matrix(), B.as_ref(), Q.as_ref());

  const auto Xt = sylvester.X.transpose();
  itkAssertOrThrowMacro(5 == Xt.rows(), "");
  itkAssertOrThrowMacro(N+1 == Xt.cols(), "");

  return Xt;

}

// This corresponds to W1 in Stam's paper.
template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix_fixed<TReal,5,5>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateS12Eigenvectors()
{

  const TReal data[5*5] = {0, -1,  1,  0,  0,
                           1, -1,  1,  0,  1,
                           1,  0,  0,  0,  0,
                           0,  0,  1,  1,  0,
                           0,  1,  0,  0,  0};
  vnl_matrix_fixed<TReal,5,5> s(data);
  return s;

}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateAEigenvectors(const unsigned int &N)
{
  vnl_matrix<TReal> s(N+6,N+6);
  s.fill(TReal(0));
  s.update(Self::CalculateSEigenvectors(N).as_ref());
  s.update(Self::CalculateS11Eigenvectors(N).as_ref(),N+1,0);
  s.update(Self::CalculateS12Eigenvectors().as_ref(),N+1,N+1);
  return s;
}

template< typename TReal, unsigned int MinN, unsigned int MaxN >
std::tuple<vnl_vector<unsigned int>, vnl_matrix<TReal>, vnl_matrix<TReal>>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateSortedEigensystem(const unsigned int &N)
{

  using TIndex = unsigned int;
  using TSort = vnl_index_sort<TReal, TIndex>;

  const auto rightEigenvectors = Self::CalculateAEigenvectors(N);
  const auto eigenvalues       = Self::CalculateAEigenvalues(N);

  // Return values are unsorted.
  // vnl_index_sort sorts from smallest to largest.
  TSort sort;
  vnl_vector<TReal>  sortedEigenvalues;
  vnl_vector<TIndex> sortedEigenvalueIndices;
  sort.vector_sort(eigenvalues, sortedEigenvalues, sortedEigenvalueIndices);
  sortedEigenvalues.flip();
  sortedEigenvalueIndices.flip();

  const auto sortedRightEigenvectors = rightEigenvectors.get_columns(sortedEigenvalueIndices);

  const auto sortedLeftEigenvectors = vnl_matrix_inverse<TReal>(sortedRightEigenvectors).inverse().transpose();

  return std::make_tuple(sortedEigenvalueIndices,
                         sortedRightEigenvectors,
                         sortedLeftEigenvectors);

}

////////////
// Bezier //
////////////

template< typename TReal, unsigned int MinN, unsigned int MaxN >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMatrices<TReal,MinN,MaxN>
::CalculateBSplineToBezierMatrix(const unsigned &N)
{

  const auto A = Self::CalculateA(N); // Extended subdivision matrix.
  const auto B = Self::CalculateB(N); // "Bigger" subdivision matrix.

  ////////////////////////////////////////
  // Calculate Eigenvalues/Eigenvectors //
  ////////////////////////////////////////

  const auto eigensystem = Self::CalculateSortedEigensystem(N);
  const auto sortedRightEigenvectors = std::get<1>(eigensystem);
  const auto sortedLeftEigenvectors  = std::get<2>(eigensystem);

  //////////////////////
  // Component Matrix //
  //////////////////////

  const TReal componentWeightData[15*13] = {
// Column numbers, for reference:
//  0   1   2   3   4   5   6   7   8   9  10  11  12
    0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // p0
    0,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // p1
   12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // p2
    0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0, // p0-
    0, 12,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0, // p0+
    0,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3, // p1-
    0,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0, // p1+
   12,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0, // p2-
   12,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0, // p2+
    0,-10,-10,  0, 32,  0,  0,  0,  0, -2,  0,  0, -2, // p01
  -10,  0,-10,  0,  0, 32,  0,  0, -2,  0,  0, -2,  0, // p12
  -10,-10,  0, 32,  0,  0,  0, -2,  0,  0, -2,  0,  0, // p20
   -7, -1, -7,  0,  0,  0, 27, -1, -1,  0,  0, -1, -1, // p0_
   -7, -7, -1,  0,  0,  0, 27, -1, -1, -1, -1,  0,  0, // p1_
   -1, -7, -7,  0,  0,  0, 27,  0,  0, -1, -1, -1, -1};// p2_

  // Component Key:
  // 0:  Patch corner X(1,0,0) //
  // 1:  Patch corner X(0,1,0) //
  // 2:  Patch corner X(0,0,1) //
  // 3:  Patch midpoint X(0.5,0.5,0.0) //
  // 4:  Patch midpoint X(0.0,0.5,0.5) //
  // 5:  Patch midpoint X(0.5,0.0,0.5) //
  // 6:  Patch center X(1/3,1/3,1/3) //
  // 7:  Xs(1,0,0) - Xr(1,0,0) //
  // 8:  Xt(1,0,0) - Xr(1,0,0) //
  // 9:  Xt(0,1,0) - Xs(0,1,0) //
  // 10: Xr(0,1,0) - Xs(0,1,0) //
  // 11: Xr(0,0,1) - Xt(0,0,1) //
  // 12: Xs(0,0,1) - Xt(0,0,1) //

  // Rows: Number of components
  // Columns: Number of points
  vnl_matrix< TReal > componentWeights(componentWeightData, 15, 13);
  componentWeights /= 12.0;

  vnl_matrix<TReal> components(13, N + 6, TReal(0.0));

  ///////////////////////////
  // Patch corner X(1,0,0) //
  ///////////////////////////

  {
    const auto v1R = sortedRightEigenvectors.get_column(0);
    itkAssertOrThrowMacro(((v1R.sum() / v1R.size()) - v1R[0]) < 10e-6, "");
    const auto v1L = sortedLeftEigenvectors.get_column(0) * v1R.get(0);

    components.set_row(0,v1L);
  }

  ///////////////////////////
  // Patch corner X(0,1,0) //
  ///////////////////////////

  {
    const TParameters p(1,0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto k =      Self::CalculateChildIndex(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,k);
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;

    components.set_row(1,conv);
  }

  ///////////////////////////
  // Patch corner X(0,0,1) //
  ///////////////////////////

  {
    const TParameters p(0,1);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;
    components.set_row(2,conv);
  }

  ///////////////////////////////////
  // Patch midpoint X(0.5,0.5,0.0) //
  ///////////////////////////////////

  {
    const TParameters p(0.5,0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;
    components.set_row(3,conv);
  }

  ///////////////////////////////////
  // Patch midpoint X(0.0,0.5,0.5) //
  ///////////////////////////////////

  {
    const TParameters p(0.5,0.5);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;
    components.set_row(4,conv);
  }

  ///////////////////////////////////
  // Patch midpoint X(0.5,0.0,0.5) //
  ///////////////////////////////////

  {
    const TParameters p(0.0,0.5);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;
    components.set_row(5,conv);
  }

  /////////////////////////////////
  // Patch center X(1/3,1/3,1/3) //
  /////////////////////////////////

  {
    const TParameters p(1./3,1./3);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto bw =     Self::CalculateBarycentricBSplineWeights(pt);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto conv = (picker*B*vnl_power(A,l-1)).transpose()*bw;
    components.set_row(6,conv);
  }

  ///////////
  // Sigma //
  ///////////

  // Right eigenvectors corresponding to subdominant eigenvalues.
  // Needed for next two components.
  const auto v2 = sortedRightEigenvectors.get_column(1);
  const auto v3 = sortedRightEigenvectors.get_column(2);

  vnl_matrix<TReal> v2v3(N+6,2,0.0);
  v2v3.set_column(0,v2);
  v2v3.set_column(1,v3);

  // See unlabeled equation, Stebbing, p. 157
  vnl_matrix<TReal> vs(1,N+6,0.0);
  for (unsigned int i = 0; i < N; ++i)
    vs.put(0,i+1,std::cos(2*M_PI*i/N));

  // See unlabeled equation, Stebbing, p. 157
  vnl_matrix<TReal> vt(1,N+6,0.0);
  for (unsigned int i = 1; i <= N; ++i)
    vt.put(0,i,std::cos(2*M_PI*i/N));

  const auto dot = vs*v2v3;

  // Ensure that one is near zero, the other isn't.
  itkAssertOrThrowMacro((dot.get(0,0) < 10e-6) != (dot.get(0,1) < 10e-6), "");

  // Find out which one is near zero.
  const unsigned int sigma_index = ((dot.get(0,0) < 10e-6) ? 1 : 0);

  // Get scaling factor.
  const auto sigma = dot.get(0,sigma_index);

  ///////////////////////////
  // Xs(1,0,0) - Xr(1,0,0) //
  ///////////////////////////

  components.set_row(7,vs.get_row(0) / sigma);

  ///////////////////////////
  // Xt(1,0,0) - Xr(1,0,0) //
  ///////////////////////////

  components.set_row(8,vt.get_row(0) / sigma);

  ///////////////////////////
  // Xt(0,1,0) - Xs(0,1,0) //
  ///////////////////////////

  {
    const TParameters p(1.0, 0.0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto bwt = Self::CalculateBarycentricBSplineWeightsDT(pt);
    const auto Xt = (picker*B*vnl_power(A,l-1)).transpose()*bwt;

    const auto bws = Self::CalculateBarycentricBSplineWeightsDS(pt);
    const auto Xs = (picker*B*vnl_power(A,l-1)).transpose()*bws;
    auto conv = Xt - Xs;

    components.set_row(9,conv);
  }

  ///////////////////////////
  // Xr(0,1,0) - Xs(0,1,0) //
  ///////////////////////////

  {
    const TParameters p(1.0, 0.0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto bwr = Self::CalculateBarycentricBSplineWeightsDR(pt);
    const auto Xr = (picker*B*vnl_power(A, l-1)).transpose()*bwr;

    const auto bws = Self::CalculateBarycentricBSplineWeightsDS(pt);
    const auto Xs = (picker*B*vnl_power(A, l-1)).transpose()*bws;
    auto conv = Xr - Xs;

    components.set_row(10,conv);
  }

  ///////////////////////////
  // Xr(0,0,1) - Xt(0,0,1) //
  ///////////////////////////

  {
    const TParameters p(0.0, 1.0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto bwr = Self::CalculateBarycentricBSplineWeightsDR(pt);
    const auto Xr = (picker*B*vnl_power(A, l-1)).transpose()*bwr;

    const auto bwt = Self::CalculateBarycentricBSplineWeightsDT(pt);
    const auto Xt = (picker*B*vnl_power(A, l-1)).transpose()*bwt;
    auto conv = Xr - Xt;

    components.set_row(11,conv);
  }

  ///////////////////////////
  // Xs(0,0,1) - Xt(0,0,1) //
  ///////////////////////////

  {
    const TParameters p(0.0, 1.0);
    const auto pt =     Self::TransformParametersToPatch(p);
    const auto picker = Self::CalculateP(N,Self::CalculateChildIndex(p));
    const auto l =      Self::CalculateNumberOfRequiredSubdivisions(p);

    const auto bws = Self::CalculateBarycentricBSplineWeightsDS(pt);
    const auto Xs = (picker*B*vnl_power(A, l-1)).transpose()*bws;

    const auto bwt = Self::CalculateBarycentricBSplineWeightsDT(pt);
    const auto Xt = (picker*B*vnl_power(A, l-1)).transpose()*bwt;
    auto conv = Xs - Xt;

    components.set_row(12,conv);
  }

  //////////////////////
  // Component Matrix //
  //////////////////////

  const auto calculatedBSplineWeights = (componentWeights*components);

  return calculatedBSplineWeights;

}

} // namespace itk

#endif
