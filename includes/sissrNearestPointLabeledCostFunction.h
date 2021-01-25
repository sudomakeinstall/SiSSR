#ifndef sissr_NearestPointLabeledCostFunction_h
#define sissr_NearestPointLabeledCostFunction_h

#include <sissrCostFunctionBase.h>

namespace sissr {

  template<class TFixedMesh, class TMovingMesh>
  class NearestPointLabeledCostFunction :
  public CostFunctionBase<TFixedMesh, TMovingMesh>
{

  public:

  using Superclass = CostFunctionBase<TFixedMesh, TMovingMesh>;
  using typename Superclass::TLocatorPointer;
  using typename Superclass::TMovingMeshPointer;
  using typename Superclass::TMovingContainerPointer;
  using typename Superclass::TLocator;
  using typename Superclass::TFixedPoint;
  using typename Superclass::TMovingPoint;
  using typename Superclass::TMovingLabel;
  using typename Superclass::TLocatorMap;

  NearestPointLabeledCostFunction(
    const TLocatorMap &_locatorMap,
    const TMovingMeshPointer &_moving,
    const TMovingContainerPointer &_initialPoints,
    unsigned int _index) :
    locatorMap(_locatorMap),
    Superclass(_moving, _initialPoints, _index)
  {}

  private:

  const TLocatorMap &locatorMap;
  TFixedPoint GetClosestPoint(const TMovingPoint &point, const TMovingLabel &label) const {
    const auto fixedPointID = this->locatorMap.at(label)->FindClosestPoint(point);
    const auto fixedPoint = this->locatorMap.at(label)->GetPoints()->ElementAt(fixedPointID);
    return fixedPoint;
  }

};

}

#endif
