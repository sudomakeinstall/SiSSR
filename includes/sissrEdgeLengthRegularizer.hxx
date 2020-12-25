#ifndef sissr_EdgeLengthRegularizer_hxx
#define sissr_EdgeLengthRegularizer_hxx

#include <sissrEdgeLengthRegularizer.h>

#include <ceres/ceres.h>
#include <itkMacro.h>
#include <limits>

namespace sissr {

template<class TMesh>
EdgeLengthRegularizer<TMesh>::EdgeLengthRegularizer(
  const typename TMesh::Pointer& _moving,
  const typename TMesh::PointsContainer::Pointer& _initialPoints,
  unsigned int _index)
  : moving(_moving)
  , initialPoints(_initialPoints)
  , index(_index)
{
  const auto edge = this->moving->GetEdge(this->index);
  this->point_indices[0] = edge->GetOrigin();
  this->point_indices[1] = edge->GetDestination();

  for (unsigned int i = 0; i < 2; ++i) {
    this->mutable_parameter_block_sizes()->push_back(
      TMesh::PointType::Dimension);
  }

  this->set_num_residuals(TMesh::PointType::Dimension);
}

template<class TMesh>
bool
EdgeLengthRegularizer<TMesh>::Evaluate(const double* const* parameters,
                                       double* residuals,
                                       double** jacobians) const
{

  const auto dim = TMesh::PointType::Dimension;

  //////////////////////////////////////////////////
  // Ensure that the point positions are updated. //
  //////////////////////////////////////////////////

  for (unsigned int i = 0; i < this->point_indices.size(); ++i) {
    const auto init = this->initialPoints->ElementAt(point_indices[i]);
    typename TMesh::PointType current;
    for (unsigned int d = 0; d < dim; ++d) {
      current[d] = init[d] + parameters[i][d];
    }
    this->moving->SetPoint(this->point_indices[i], current);
  }

  /////////////////////////
  // Calculate Residuals //
  /////////////////////////

  const auto pt1 = this->moving->GetPoint(this->point_indices[0]);
  const auto pt2 = this->moving->GetPoint(this->point_indices[1]);

  for (unsigned int i = 0; i < dim; ++i) {
    residuals[i] = pt2[i] - pt1[i];
  }

  // Continue if not requested
  if (nullptr == jacobians)
    return true;

  // Set Jacobian to zero.
  for (unsigned int i = 0; i < 2; ++i) {
    if (nullptr != jacobians[i]) {
      ceres::MatrixRef(jacobians[i], dim, dim).setZero();
    }
  }

  // Set the relevant Jacobians.
  for (unsigned int i = 0; i < dim; ++i) {
    if (nullptr != jacobians[0])
      jacobians[0][i + i * dim] = -1;
    if (nullptr != jacobians[1])
      jacobians[1][i + i * dim] = 1;
  }

  return true;
}

} // namespace sissr

#endif
