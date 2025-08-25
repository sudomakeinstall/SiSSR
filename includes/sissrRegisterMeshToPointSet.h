#ifndef sissr_RegisterMeshToPointSet_h
#define sissr_RegisterMeshToPointSet_h

// STD
#include <vector>

// ITK
#include <itkMesh.h>
#include <itkLoopSubdivisionSurfaceMesh.h>
#include <itkPointsLocator.h>


// SiSSR
#include <sissrAccelerationRegularizer.h>
#include <sissrEdgeLengthRegularizer.h>
#include <sissrVelocityRegularizer.h>
#include <sissrTriangleAspectRatioRegularizer.h>
#include <sissrThinPlateRegularizer.h>
#include <sissrLossScaleFactors.h>
#include <sissrNearestPointLabeledCostFunction.h>
#include <sissrNearestPointUnlabeledCostFunction.h>

namespace sissr {

template < typename TFixedMesh, typename TMovingMesh >
class RegisterMeshToPointSet
{

public:

  int MaximumNumberOfIterations = 500; // Ceres default is 50
  int MaximumSolverTimeInSeconds = 60 * 60; // Default is 1e6
  double FunctionTolerance = 1e-6; // Default is 1e-6
  double ParameterTolerance = 1e-8; // Default is 1e-8
  bool DynamicSparsity = false;
  const bool UseLabels;

  LossScaleFactors RegistrationWeights;

  using TFixed = TFixedMesh;
  using TMoving = TMovingMesh;

  using TContainer = typename TFixedMesh::PointsContainer;
  using TLocator = itk::PointsLocator<TContainer>;
  using TLocatorVector = std::vector<typename TLocator::Pointer>;
  using TLocatorMapVector = std::vector<std::map<size_t, typename TLocator::Pointer>>;

  using TFixedVector = std::vector<typename TFixed::Pointer>;
  using TMovingVector = std::vector<typename TMoving::Pointer>;

  using TLabeledPrimaryResidual = NearestPointLabeledCostFunction<TFixed, TMoving>;
  using TUnlabeledPrimaryResidual = NearestPointUnlabeledCostFunction<TFixed, TMoving>;
  using TVelocityRegularizer = VelocityRegularizer<TMoving>;
  using TAccelerationRegularizer = AccelerationRegularizer<TMoving>;
  using TThinPlateRegularizer = ThinPlateRegularizer<TMoving>;
  using TTriangleAspectRatioRegularizer = TriangleAspectRatioRegularizer<TMoving>;
  using TEdgeLengthRegularizer = EdgeLengthRegularizer<TMoving>;
  using TParameterVector = std::vector<std::vector<double*>>;

  RegisterMeshToPointSet(const TFixedVector &_fixedVector,
                         const TMovingVector &_movingVector,
                         const bool &_UseLabels);

  TLocatorVector locatorVector;
  TLocatorMapVector locatorMapVector;
  const TMovingVector movingVector;

  void SanityCheck();

  unsigned int CalculateNumberOfFrames() const;
  unsigned int CalculateNumberOfControlPoints() const;
  unsigned int CalculateNumberOfSurfacePoints() const;
  unsigned int CalculateNumberOfCells() const;

  const unsigned int NumberOfFrames;
  const unsigned int NumberOfControlPoints;
  const unsigned int NumberOfSurfacePoints;
  const unsigned int NumberOfCells;

  void Register();
  void AddLabeledPrimaryResidual(ceres::Problem&, TParameterVector&);
  void AddUnlabeledPrimaryResidual(ceres::Problem&, TParameterVector&);
  void AddVelocityRegularizer(ceres::Problem&, TParameterVector&);
  void AddAccelerationRegularizer(ceres::Problem&, TParameterVector&);
  void AddThinPlateRegularizer(ceres::Problem&, TParameterVector&);
  void AddTriangleAspectRatioRegularizer(ceres::Problem&, TParameterVector&);
  void AddEdgeLengthRegularizer(ceres::Problem&, TParameterVector&);

  std::vector<double>                 costFunctionResiduals;
  std::vector<ceres::ResidualBlockId> costFunctionResidualIDs;
  std::vector<unsigned int>           costFunctionCellIDs;
  std::vector<unsigned int>           costFunctionFrames;
  std::string summaryString;

  std::vector<typename TMoving::PointsContainer::Pointer> initialPointsVector;
};

} // namespace sissr

#include "sissrRegisterMeshToPointSet.hxx"

#endif
