// std
#include <utility>
#include <tuple>
#include <iostream>

// ITK
#include <itkLoopSubdivisionSurfaceMatrices.h>
#include <itkTestingMacros.h>
#include <itkExampleLoopMeshSource.h>
#include <itkQuadEdgeMesh.h>

// VXL
#include <vnl/vnl_matrix_fixed.hxx>
#include <vnl/vnl_math.h>
#include <vnl/vnl_real.h>
#include <vnl/vnl_imag.h>
#include <vnl/algo/vnl_real_eigensystem.h>
#include <vnl/algo/vnl_matrix_inverse.h>

int main(int itkNotUsed(argc), char * itkNotUsed(argv) [])
{

  const unsigned int N = 6;
  const unsigned int minN = 4;
  const unsigned int maxN = 10;

  typedef double                                                   TReal;
  typedef itk::LoopSubdivisionSurfaceMatrices< TReal, minN, maxN > TMatrices;
  TMatrices matrices;

  const TReal eps = 10e-5;

  ////////////////////////////////////
  // Sanity Check -- Print Out Mats //
  ////////////////////////////////////

  std::cout << "Source matrices:" << '\n';

  std::cout << "S:\n" << matrices.GetS(N) << '\n';
  std::cout << "A:\n" << matrices.GetA(N) << '\n';
  std::cout << "B:\n" << matrices.GetB(N) << '\n';

  std::cout << "Eigenvalues:" << '\n';

  std::cout << "S:\n" << matrices.GetSEigenvalues(N) << '\n';
  std::cout << "A:\n" << matrices.GetAEigenvalues(N) << '\n';

  std::cout << "Eigenvectors:" << '\n';

  std::cout << "S:\n" << matrices.GetSEigenvectors(N) << '\n';
  std::cout << "A:\n" << matrices.GetAEigenvectors(N) << '\n';

  std::cout << "Thin plate energy:" << '\n';

  std::cout << "BSplineM:\n"       << matrices.GetBSplineM()     << '\n';
  std::cout << "sqrt(BSplineM):\n" << matrices.GetBSplineMRoot() << '\n';
  std::cout << "BezierM:\n"        << matrices.GetBezierM()     << '\n';
  std::cout << "sqrt(BezierM):\n"  << matrices.GetBezierMRoot() << '\n';

    {
    // BSpline
    const auto M = matrices.GetBSplineM();
    const auto M_root_squared = matrices.GetBSplineMRoot()*matrices.GetBSplineMRoot();
    itkAssertOrThrowMacro((M - M_root_squared).array_two_norm() < 10e-6, "");
    }

    {
    // Bezier
    const auto M = matrices.GetBezierM();
    const auto M_root_squared = matrices.GetBezierMRoot()*matrices.GetBezierMRoot();
    itkAssertOrThrowMacro((M - M_root_squared).array_two_norm() < 10e-6, "");
    }

  //////////////////////////
  // CREATE A TEST MATRIX //
  //////////////////////////

  
  typedef itk::QuadEdgeMesh< float, 3 >       TMesh;
  typedef itk::ExampleLoopMeshSource< TMesh > TSource;
  const auto exampleMesh = TSource::New();
  exampleMesh->Update();
  const auto testPoints = exampleMesh->GetPointsAsVnlMatrix();

  std::cout << "BSpline Test Points:\n" << testPoints << '\n';

  const vnl_matrix_fixed<TReal,36,1> TestPointsCol(testPoints.transpose().data_block());
  const auto TestPointsRow = TestPointsCol.transpose();

  std::cout << TestPointsCol << std::endl;
  std::cout << TestPointsRow << std::endl;

  const auto P = matrices.GetBSplineToBezier(6);
  const auto BezierTestPoints = P * testPoints;
  const vnl_matrix_fixed<TReal,45,1> BezierTestPointsCol(BezierTestPoints.transpose().data_block());
  const auto BezierTestPointsRow = BezierTestPointsCol.transpose();

  ///////////////////////////////////////////////
  // Check that the thin plate energy is zero. //
  ///////////////////////////////////////////////

    {
    // The normal way
    const auto M_block = matrices.GetBSplineMBlock();
    const auto tpe = (TestPointsRow*M_block*TestPointsCol)(0,0);
  
    std::cout << "Total Thin Plate Energy: " << tpe << std::endl;
    if (eps < tpe)
      {
      std::cerr << "ERROR: Thin plate energy should be near zero." << std::endl;
      return EXIT_FAILURE;
      }
  
    // The root way
    const auto M_root_block = matrices.GetBSplineMRootBlock();
    const auto tpv = M_root_block*TestPointsCol;
    std::cout << "Thin plate energy vector: " << tpv << std::endl;
  
    for (unsigned int i = 0; i < tpv.rows(); ++i)
      {
      if (eps < std::fabs(tpv.get(i,0)))
        {
        std::cerr << "ERROR: All components should be near zero." << std::endl;
        return EXIT_FAILURE;
        }
      }
    }

  ////////////////////////////
  // Same thing, but Bezier //
  ////////////////////////////

    {
    // The normal way
    const auto M_block = matrices.GetBezierMBlock();
    const auto tpe = (BezierTestPointsRow*M_block*BezierTestPointsCol)(0,0);
  
    std::cout << "Total Thin Plate Energy: " << tpe << std::endl;
    if (eps < tpe)
      {
      std::cerr << "ERROR: Thin plate energy should be near zero." << std::endl;
      return EXIT_FAILURE;
      }
  
    // The root way
    const auto M_root_block = matrices.GetBezierMRootBlock();
    const auto tpv = M_root_block*BezierTestPointsCol;
    std::cout << "Thin plate energy vector: " << tpv << std::endl;
  
    for (unsigned int i = 0; i < tpv.rows(); ++i)
      {
      if (eps < std::fabs(tpv.get(i,0)))
        {
        std::cerr << "ERROR: All components should be near zero." << std::endl;
        return EXIT_FAILURE;
        }
      }
    }


  /////////////////////////////
  // Test Utility functions. //
  /////////////////////////////

  // Far edge.
  const double third = 1.0/3.0;
  auto p = std::pair<TReal,TReal>(third,third);
  unsigned int n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  unsigned int i = matrices.CalculateChildIndex(p);
  auto picker = matrices.GetP(N,i);
  bool verified = matrices.VerifyParameters(p);
  auto weights = matrices.CalculateBarycentricBSplineWeights(p);
  auto bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum()-1.0) < 10e-5,"");
  itkAssertOrThrowMacro(std::abs(bweights.sum()-1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 1: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((1 == n && 1 == i && verified),"Test 1");

  p.first  = third + 0.2;
  p.second = third - 0.2;
  n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  i = matrices.CalculateChildIndex(p);
  picker = matrices.GetP(N,i);
  verified = matrices.VerifyParameters(p);
  weights = matrices.CalculateBarycentricBSplineWeights(p);
  bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum() - 1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 2: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((1 == n && 0 == i && verified),"Test 2");

  p.first  = third - 0.2;
  p.second = third + 0.2;
  n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  i = matrices.CalculateChildIndex(p);
  picker = matrices.GetP(N,i);
  verified = matrices.VerifyParameters(p);
  weights = matrices.CalculateBarycentricBSplineWeights(p);
  bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum() - 1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 3: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((1 == n && 2 == i && verified),"Test 3");

  // A little closer
  p.first  = third - 0.1;
  p.second = third - 0.1;
  n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  i = matrices.CalculateChildIndex(p);
  picker = matrices.GetP(N,i);
  verified = matrices.VerifyParameters(p);
  weights = matrices.CalculateBarycentricBSplineWeights(p);
  bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum() - 1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 4: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((2 == n && 1 == i && verified),"Test 4");

  p.first  = third - 0.0;
  p.second = third - 0.2;
  n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  i = matrices.CalculateChildIndex(p);
  picker = matrices.GetP(N,i);
  verified = matrices.VerifyParameters(p);
  weights = matrices.CalculateBarycentricBSplineWeights(p);
  bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum() - 1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 5: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((2 == n && 0 == i && verified),"Test 5");

  p.first  = third - 0.2;
  p.second = third - 0.0;
  n = matrices.CalculateNumberOfRequiredSubdivisions(p);
  i = matrices.CalculateChildIndex(p);
  picker = matrices.GetP(N,i);
  verified = matrices.VerifyParameters(p);
  weights = matrices.CalculateBarycentricBSplineWeights(p);
  bweights = matrices.CalculateBarycentricBSplineWeights(p);
  itkAssertOrThrowMacro(std::abs(weights.sum() - 1.0) < 10e-5,"");
  for (unsigned int id = 0; id < weights.size(); ++id)
    itkAssertOrThrowMacro(std::abs(weights.get(id) - bweights.get(id)) < 10e-5,"");
  std::cout << "Test 6: " << n << " " << i << " " << verified << std::endl;
  itkAssertOrThrowMacro((2 == n && 2 == i && verified),"Test 6");


  const auto A = matrices.GetA(10);
  std::cout << A << std::endl;

  // Ensure that the rows of A all sum to one.

  for (unsigned int r = 0; r < A.rows(); ++r)
    {
    if (1.0 - A.get_row(i).sum() > 10e-5)
      {
      std::cerr << "Row "
                << i
                << " of the extended subdivision matrix doesn't sum to one."
                << std::endl;
      return EXIT_FAILURE;
      }
    }

  const auto S = vnl_real_eigensystem(A);
  const auto R = vnl_real(S.V).transpose(); // Columnwise
  const auto L = vnl_svd<TReal>(R).inverse().transpose(); // Columnwise
  const auto index = vnl_real(S.D.diagonal()).apply(std::abs).arg_max();

  std::cout << "R: " << R.get_row(index).sum() << std::endl;
  std::cout << "L: " << L.get_row(index).sum() << std::endl;

  ////////////////////////////////////////////////
  // BARYCENTRIC WEIGHTS FOR BEZIER COORDINATES //
  ////////////////////////////////////////////////

  const auto corner010 = matrices.CalculateBarycentricBSplineWeights(std::make_pair(1.0,0.0));
  const auto corner001 = matrices.CalculateBarycentricBSplineWeights(std::make_pair(0.0,1.0));
  (void)corner010;
  (void)corner001;

  const auto midpoint550 = matrices.CalculateBarycentricBSplineWeights(std::make_pair(0.5,0.0));
  const auto midpoint055 = matrices.CalculateBarycentricBSplineWeights(std::make_pair(0.5,0.5));
  const auto midpoint505 = matrices.CalculateBarycentricBSplineWeights(std::make_pair(0.0,0.5));
  (void)midpoint550;
  (void)midpoint055;
  (void)midpoint505;

  const auto center = matrices.CalculateBarycentricBSplineWeights(std::make_pair(1./3.,1./3.));
  (void)center;

  const auto diffTS = matrices.CalculateBarycentricBSplineWeightsDT(std::make_pair(1.0,0.0)) -
                      matrices.CalculateBarycentricBSplineWeightsDS(std::make_pair(1.0,0.0));
  const auto diffRS = matrices.CalculateBarycentricBSplineWeightsDR(std::make_pair(1.0,0.0)) -
                      matrices.CalculateBarycentricBSplineWeightsDS(std::make_pair(1.0,0.0));
  (void)diffTS;
  (void)diffRS;

  const auto diffRT = matrices.CalculateBarycentricBSplineWeightsDR(std::make_pair(0.0,1.0)) -
                      matrices.CalculateBarycentricBSplineWeightsDT(std::make_pair(0.0,1.0));
  const auto diffST = matrices.CalculateBarycentricBSplineWeightsDS(std::make_pair(0.0,1.0)) -
                      matrices.CalculateBarycentricBSplineWeightsDT(std::make_pair(0.0,1.0));
  (void)diffRT;
  (void)diffST;

  //////////////////////////////
  // Eigenvalues/Eigenvectors //
  //////////////////////////////

  const auto sub = matrices.GetS(N);
  const auto vec = matrices.GetSEigenvectors(N);
  const auto val = matrices.GetSEigenvalues(N);

  itkAssertOrThrowMacro((sub*vec - vec*vnl_diag_matrix<double>(val)).array_two_norm() < eps, "");

  return EXIT_SUCCESS;

}
