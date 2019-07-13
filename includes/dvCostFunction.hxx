#ifndef dvCostFunction_hxx
#define dvCostFunction_hxx

#include <dvCostFunction.h>

namespace dv
{
template<class TFixedMesh, class TMovingMesh>
CostFunction<TFixedMesh, TMovingMesh>
::CostFunction(
               const typename TLocator::Pointer
                 &_locator,
               const typename TMovingMesh::Pointer
                 &_moving,
               const typename TMovingMesh::PointsContainer::Pointer
                 &_initialPoints,
               unsigned int _index)
  :
    locator(_locator),
    moving(_moving),
    initialPoints(_initialPoints),
    index(_index),
    param(this->moving->GetSurfaceParameterList().at(this->index)),
    residual(this->moving->GetResidualBlock(this->param.first,this->param.second)),
    L(this->moving->GetPointListForCell(this->param.first))
{

  for (std::size_t i = 0; i < this->L.size(); ++i)
    {
    this->mutable_parameter_block_sizes()->push_back(3);
    }

  this->set_num_residuals(3);
}

template<class TFixedMesh, class TMovingMesh>
bool
CostFunction<TFixedMesh, TMovingMesh>
::Evaluate(const double* const* parameters,
           double* residuals,
           double** jacobians) const
{

  for (std::size_t i = 0; i < this->L.size(); ++i)
    {
    const auto initial = this->initialPoints->ElementAt(this->L[i]);
    const auto difference = parameters[i];
    typename TMovingMesh::PointType current;
    for (unsigned int d = 0; d < 3; ++d)
      {
      current[d] = initial[d] + difference[d];
      }
    this->moving->SetPoint( this->L[i], current );
    }      

  // Residuals
  const auto movingPoint =
    this->moving->GetPointOnSurface(this->param.first, this->param.second);
  const auto fixedPointID = this->locator->FindClosestPoint(movingPoint);
  const auto fixedPoint = this->locator->GetPoints()->ElementAt(fixedPointID);
  const auto error = movingPoint.GetVnlVector() - fixedPoint.GetVnlVector();

  for (unsigned int d = 0; d < 3; ++d) residuals[d] = error[d];

  // Return if Jacobian wasn't requested.
  if (nullptr == jacobians)
    {
    return true;
    }

  // Set Jacobian to zero.
  for (std::size_t i = 0; i < L.size(); ++i)
    {

    if (nullptr == jacobians[i]) continue;

    ceres::MatrixRef(jacobians[i],3,3).setZero();

    jacobians[i][0] = residual[i];
    jacobians[i][4] = residual[i];
    jacobians[i][8] = residual[i];

    }

  return true;

}

} // end namespace

#endif

