/*
 * The purpose of this test is to calculate the matrix M
 * which multiplies the subdivision control point matrix X
 * to yield the Bezier control point matrix, P:
 *
 * P = M*X
 *
 * The individual points (rows) of P are composed of a linear
 * combination of the following "components":
 *
 * X(1.0, 0.0, 0.0) <--
 * X(0.0, 1.0, 0.0)
 * X(0.0, 0.0, 1.0)
 *
 * X(0.5, 0.5, 0.0)
 * X(0.0, 0.5, 0.5)
 * X(0.5, 0.0, 0.5)
 *
 * X(1/3, 1/3, 1/3)
 *
 * Xs(1,0,0) - Xr(1,0,0) <--
 * Xt(1,0,0) - Xr(1,0,0) <--
 *
 * Xt(0,1,0) - Xs(0,1,0)
 * Xr(0,1,0) - Xs(0,1,0)
 *
 * Xr(0,0,1) - Xt(0,0,1)
 * Xs(0,0,1) - Xt(0,0,1)
 *
 * Note the components indicated by an arrow, which cannot be
 * directly calculated using Stam's scheme.  Calculation of these
 * values is treated in Richard Stebbing's thesis (A.1.5, A.1.9).
 *
 * The contributions of each "component" to each Bezier point has been
 * tabulated in the "componentWeights" matrix, below.
 *
 * M = componentWeights * components
 *
 */

// VNL
#include <vnl/algo/vnl_real_eigensystem.h>
#include <vnl/vnl_real.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/vnl_vector_fixed.hxx>

// ITK
#include "itkLoopSubdivisionSurfaceMesh.h"
#include "itkExampleLoopMeshSource.h"
#include "itkLoopSubdivisionSurfaceMatrices.h"
#include "itkTestingMacros.h"

const unsigned int minN = 4;
const unsigned int maxN = 10; 
const unsigned int Dimension = 3;

typedef double                                                   TReal;
typedef itk::LoopSubdivisionSurfaceMesh< TReal, Dimension >      TMesh;
typedef itk::ExampleLoopMeshSource< TMesh >                      TExamplePatch;
typedef itk::LoopSubdivisionSurfaceMatrices< TReal, minN, maxN > TMatrices;

// Set very small values to zero (easier to read when printing to std::cout)
TReal reset(TReal v) { return std::abs(v) < 10e-6 ? 0 : v; }

int main(int argc, char ** itkNotUsed(argv) )
{

  if (1 != argc)
    {
    std::cerr << "Unrecognized arguments supplied." << std::endl;
    return EXIT_FAILURE;
    }

  ////////////////////////////////////
  // Define the necessary matrices. //
  ////////////////////////////////////

  // This object contains the subdivision matrices used in subsequent calculations.
  // See itkLoopSubdivisionSurfaceMatrices.h for details.

  TMatrices matrices;

  const unsigned int N = 6;
  const auto calculatedBezierWeights = matrices.GetBSplineToBezier(N);
  std::cout << "Calculated Bezier Weights (*24): " << '\n'
           << calculatedBezierWeights*24 << '\n';

  ///////////////////////
  // Compare with A.18 //
  ///////////////////////

  TReal nSixData[15*12] = {
   2,12, 2, 0, 0, 0, 2, 2, 2, 2, 0, 0,
   2, 2, 0, 0, 0, 2,12, 2, 0, 0, 2, 2, 
  12, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
   4,12, 3, 0, 0, 0, 3, 1, 0, 1, 0, 0,
   3,12, 1, 0, 0, 0, 4, 3, 1, 0, 0, 0,
   3, 4, 0, 0, 0, 1,12, 3, 0, 0, 1, 0,
   4, 3, 0, 0, 0, 3,12, 1, 0, 0, 0, 1,
  12, 3, 1, 0, 1, 3, 4, 0, 0, 0, 0, 0,
  12, 4, 3, 1, 0, 1, 3, 0, 0, 0, 0, 0,
   4, 8, 0, 0, 0, 0, 8, 4, 0, 0, 0, 0,
   8, 4, 0, 0, 0, 4, 8, 0, 0, 0, 0, 0,
   8, 8, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0,
   6,10, 1, 0, 0, 0, 6, 1, 0, 0, 0, 0,
   6, 6, 0, 0, 0, 1,10, 1, 0, 0, 0, 0,
  10, 6, 1, 0, 0, 1, 6, 0, 0, 0, 0, 0};

  vnl_matrix<TReal> knownBezierWeights(nSixData, 15, 12);
  knownBezierWeights /= 24;

  std::cout << "Known Bezier Weights (*24): " << '\n'
           << knownBezierWeights*24 << '\n';

  std::cout << "Known*24 - Calculated*24: " << '\n'
           << (knownBezierWeights*24 - calculatedBezierWeights*24).apply(reset) << '\n';

  for (unsigned int n = minN; n <= maxN; ++n)
    std::cout << n << ":" << '\n' << matrices.GetBSplineToBezier(n).apply(reset) << '\n';

  //
  // 
  //

  const auto example = TExamplePatch::New();
  example->SetValency( N );
  example->Update();

  const auto BSplinePoints = example->GetPointsAsVnlMatrix();
  const auto BezierPoints = matrices.GetBSplineToBezier( N ) * BSplinePoints;
  std::cout << BezierPoints << '\n';

  const auto flat = BezierPoints.flatten_column_major();
  std::cout << flat << '\n';

  const auto mats = matrices.GetBezierMRootBlock();

  const auto totalTPE = dot_product(flat * mats, flat);
  itkAssertOrThrowMacro(totalTPE < 10e-6, "");

  itkAssertOrThrowMacro((mats * flat).sum() < 10e-6, "");

//  vnl_vector<TReal> v(BezierPoints.rows() * BezierPoints.cols());
//  for (unsigned int c = 0; c < BezierPoints.cols(); ++c) 
//    {
//    for (unsigned int r = 0; r < BezierPoints.rows(); ++r)
//      {
//      const auto vectorIndex = c * BezierPoints.cols() + r;
//      std::cout << vectorIndex << std::endl;
//      v.put(c*BezierPoints.rows()+r, BezierPoints.get(r, c));
//      }
//    }
//  std::cout << v << '\n';
//  std::cout << matrices.GetBezierMRootBlock() * BezierPoints << '\n';

  return EXIT_SUCCESS;

}
