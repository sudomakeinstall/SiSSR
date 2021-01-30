#ifndef sissr_NearestPointUnlabeledCostFunction_h
#define sissr_NearestPointUnlabeledCostFunction_h

#include <sissrCostFunctionBase.h>

namespace sissr {

  template<class TFixedMesh, class TMovingMesh>
  class NearestPointUnlabeledCostFunction :
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

  NearestPointUnlabeledCostFunction(
    const TLocatorPointer &_locator,
    const TMovingMeshPointer &_moving,
    const TMovingContainerPointer &_initialPoints,
    unsigned int _index) :
    locator(_locator),
    Superclass(_moving, _initialPoints, _index)
  {}

  private:

  const TLocatorPointer &locator;
  TFixedPoint GetClosestPoint(const TMovingPoint &point, const TMovingLabel &label) const {
    const auto fixedPointID = this->locator->FindClosestPoint(point);
    const auto fixedPoint = this->locator->GetPoints()->ElementAt(fixedPointID);
    return fixedPoint;
  }

};

}

#endif
