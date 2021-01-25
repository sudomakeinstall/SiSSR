#ifndef sissr_CostFunctionBase_h
#define sissr_CostFunctionBase_h

// STD
#include <limits>

// ITK
#include <itkPointsLocator.h>

// Ceres
#include <ceres/ceres.h>

namespace sissr {

template<class TFixedMesh, class TMovingMesh>
class CostFunctionBase :
public ceres::CostFunction
{

  public:

  using TFixedContainer = typename TFixedMesh::PointsContainer;
  using TFixedPoint = typename TFixedMesh::PointType;

  using TMovingMeshPointer = typename TMovingMesh::Pointer;
  using TMovingContainer = typename TMovingMesh::PointsContainer;
  using TMovingContainerPointer = typename TMovingContainer::Pointer;
  using TMovingPoint = typename TMovingMesh::PointType;
  using TMovingLabel = typename TMovingMesh::MeshTraits::CellPixelType;

  using TLocator = itk::PointsLocator< TFixedContainer >;
  using TLocatorPointer = typename TLocator::Pointer;
  using TLocatorMap = typename std::map<size_t, typename TLocator::Pointer>;

  CostFunctionBase(
    const TMovingMeshPointer &_moving,
    const TMovingContainerPointer &_initialPoints,
    unsigned int _index) :
    moving(_moving),
    initialPoints(_initialPoints),
    index(_index),
    param(this->moving->GetSurfaceParameterList().at(this->index)),
    residual(this->moving->GetResidualBlock(this->param.first,this->param.second)),
    L(this->moving->GetPointListForCell(this->param.first))
  {
    this->Setup();
  }

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~CostFunctionBase(){}

private:

  void Setup() {
    for (size_t i = 0; i < this->L.size(); ++i)
      {
      this->mutable_parameter_block_sizes()->push_back(3);
      }
    this->set_num_residuals(3);
  }

  const typename TMovingMesh::Pointer &moving;
  const typename TMovingContainer::Pointer &initialPoints;

  const unsigned int index;
  const typename TMovingMesh::TSurfaceParameter param;
  const vnl_vector<typename TMovingMesh::RealType> residual;
  const vnl_vector<typename TMovingMesh::PointIdentifier> L;

  virtual TFixedPoint GetClosestPoint(const TMovingPoint &point, const TMovingLabel &label) const = 0;

}; // end class

} // namespace sissr

#include <sissrCostFunctionBase.hxx>

#endif
