#ifndef sissr_CostFunctionBase_hxx
#define sissr_CostFunctionBase_hxx

#include <sissrCostFunctionBase.h>

namespace sissr {

template<class TFixedMesh, class TMovingMesh>
bool
CostFunctionBase<TFixedMesh, TMovingMesh>
::Evaluate(const double* const* parameters,
           double* residuals,
           double** jacobians) const
{

  for (size_t i = 0; i < this->L.size(); ++i)
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
  const auto cellID = std::get<0>(this->moving->GetSurfaceParameter(this->index));
  const auto label = this->moving->GetCellData()->ElementAt(cellID);
  itkAssertOrThrowMacro(label != 0, "Label == 0");
  const auto fixedPoint = this->GetClosestPoint(movingPoint, label);
  const auto error = movingPoint.GetVnlVector() - fixedPoint.GetVnlVector();

  for (unsigned int d = 0; d < 3; ++d) residuals[d] = error[d];

  // Return if Jacobian wasn't requested.
  if (nullptr == jacobians)
    {
    return true;
    }

  // Set Jacobian to zero.
  for (size_t i = 0; i < L.size(); ++i)
    {

    if (nullptr == jacobians[i]) continue;

    ceres::MatrixRef(jacobians[i],3,3).setZero();

    jacobians[i][0] = residual[i];
    jacobians[i][4] = residual[i];
    jacobians[i][8] = residual[i];

    }

  return true;

}

} // namespace sissr

#endif
