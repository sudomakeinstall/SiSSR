#ifndef sissr_RegisterMeshToPointSet_h
#define sissr_RegisterMeshToPointSet_h

// STD
#include <vector>

// ITK
#include <itkMesh.h>
#include <itkLoopSubdivisionSurfaceMesh.h>
#include <itkPointsLocator.h>

// Custom
#include <dvProgress.h>

// SiSSR
#include <sissrAccelerationRegularizer.h>
#include <sissrEdgeLengthRegularizer.h>
#include <sissrVelocityRegularizer.h>
#include <sissrTriangleAspectRatioRegularizer.h>
#include <sissrThinPlateRegularizer.h>
#include <sissrLossScaleFactors.h>
#include <sissrNearestPointLabeledCostFunction.h>

namespace sissr {

template < typename TFixedMesh, typename TMovingMesh >
class RegisterMeshToPointSet
{

public:

  int MaximumNumberOfIterations = 15; // Default is 50
  int MaximumSolverTimeInSeconds = 60 * 60; // Default is 1e6
  double FunctionTolerance = 1e-2;
  double ParameterTolerance = 1e-2;
  bool DynamicSparsity = false;

  LossScaleFactors RegistrationWeights;

  typedef TFixedMesh TFixed;
  typedef TMovingMesh TMoving;

  typedef typename TFixedMesh::PointsContainer TContainer;
  typedef itk::PointsLocator< TContainer > TLocator;
  typedef std::vector<typename TLocator::Pointer> TLocatorVector;
  typedef std::vector<std::map<size_t, typename TLocator::Pointer>> TLocatorMapVector;

  typedef std::vector<typename TFixed::Pointer> TFixedVector;
  typedef std::vector<typename TMoving::Pointer> TMovingVector;

  typedef NearestPointLabeledCostFunction<TFixed,TMoving> TCost;
  typedef VelocityRegularizer<TMoving> TVelocityRegularizer;
  typedef AccelerationRegularizer<TMoving> TAccelerationRegularizer;
  typedef ThinPlateRegularizer<TMoving> TThinPlateRegularizer;
  typedef TriangleAspectRatioRegularizer<TMoving> TTriangleAspectRatioRegularizer;
  typedef EdgeLengthRegularizer<TMoving> TEdgeLengthRegularizer;
  using TParameterVector = std::vector<std::vector<double*>>;

  RegisterMeshToPointSet(const unsigned int& _EDFrame,
                         const TFixedVector &_fixedVector,
                         const TMovingVector &_movingVector);

  const unsigned int EDFrame;
  const TLocatorVector locatorVector;
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
  void AddPrimaryCost(ceres::Problem&, TParameterVector&);
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
