#include <ceres/loss_function.h>
#include <dvMath.h>
#include <dvTriangleAspectRatioRegularizer.h>
#include <itkMesh.h>
#include <itkTriangleCell.h>

typedef itk::Mesh<double, 3> TMesh;
typedef itk::TriangleCell<TMesh::CellType> TTriangle;
typedef dv::TriangleAspectRatioRegularizer<TMesh> TReg;

int
main(int, char**)
{

  ///////////////////////////////////////////////////////
  // Create a mesh with a single equalateral triangle. //
  ///////////////////////////////////////////////////////

  TMesh::PointType p0, p1, p2;
  p0[0] = -0.5, p0[1] = 0.0, p0[2] = 0.0;
  p1[0] = 0.0, p1[1] = std::sqrt(1.0 - 0.5 * 0.5), p1[2] = 0.0;
  p2[0] = +0.5, p2[1] = 0.0, p2[2] = 0.0;

  // All edge lengths should be 1.  (Aspect ratios also 1.)
  assert(dv::Close(p0.EuclideanDistanceTo(p1), 1.0));
  assert(dv::Close(p1.EuclideanDistanceTo(p2), 1.0));
  assert(dv::Close(p2.EuclideanDistanceTo(p0), 1.0));

  const auto mesh = TMesh::New();
  mesh->SetPoint(0, p0);
  mesh->SetPoint(1, p1);
  mesh->SetPoint(2, p2);

  TMesh::CellAutoPointer cell;
  cell.TakeOwnership(new TTriangle);
  cell->SetPointId(0, 0);
  cell->SetPointId(1, 1);
  cell->SetPointId(2, 2);

  mesh->SetCell(0, cell);

  ///////////////////////////
  // Set up cost function. //
  ///////////////////////////

  const auto init = TMesh::PointsContainer::New();
  for (auto it = mesh->GetPoints()->Begin(); it != mesh->GetPoints()->End();
       ++it) {
    init->InsertElement(it.Index(), it.Value());
  }

  ceres::CostFunction* reg = new TReg(mesh, init, 0);

  //////////////////////////
  // Allocate parameters. //
  //////////////////////////

  std::vector<double*> parameters;
  for (unsigned int i = 0; i < 3; ++i) {
    parameters.push_back(new double[3]);
    parameters[i][0] = 0.0;
    parameters[i][1] = 0.0;
    parameters[i][2] = 0.0;
  }

  double residuals[1] = { 0.0 };
  double residuals_eps[1] = { 0.0 };

  double** jacobians = new double*[3];
  for (unsigned int i = 0; i < 3; ++i)
    jacobians[i] = new double[3];

  double eps = 10e-6;

  ///////////////////////////////////////////////////
  // Ensure that residuals are near zero to start. //
  ///////////////////////////////////////////////////

  reg->Evaluate(parameters.data(), residuals, nullptr);
  assert(dv::Close(residuals[0], 0.0));

  ///////////
  // p0, x //
  ///////////

  for (unsigned int p = 0; p < 3; ++p) {
    for (unsigned int d = 0; d < 3; ++d) {
      ceres::VectorRef(parameters[0], 3).setZero();
      ceres::VectorRef(parameters[1], 3).setZero();
      ceres::VectorRef(parameters[2], 3).setZero();

      parameters[p][d] = eps;

      reg->Evaluate(parameters.data(), residuals_eps, jacobians);

      {
        const auto observed = (residuals_eps[0] - residuals[0]) / eps;
        const auto calculated = jacobians[p][d];
        assert(dv::Close(observed, calculated, 0.01));
      }
    }
  }

  return EXIT_SUCCESS;
}
