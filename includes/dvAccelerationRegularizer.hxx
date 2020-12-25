
#ifndef dvAccelerationRegularizer_hxx
#define dvAccelerationRegularizer_hxx

#include <ceres/ceres.h>
#include <limits>

namespace sissr {

template<class TMovingMesh>
AccelerationRegularizer<TMovingMesh>::AccelerationRegularizer(
  const typename std::vector<typename TMovingMesh::Pointer>& _movingVector,
  const typename std::vector<typename TMovingMesh::PointsContainer::Pointer>&
    _initialPointsVector,
  unsigned int _frame,
  unsigned int _index)
  : movingVector(_movingVector)
  , initialPointsVector(_initialPointsVector)
  , frame(_frame)
  , index(_index)
{
  const auto dim = TMovingMesh::PointType::Dimension;
  this->mutable_parameter_block_sizes()->push_back(dim);
  this->mutable_parameter_block_sizes()->push_back(dim);
  this->mutable_parameter_block_sizes()->push_back(dim);
  this->set_num_residuals(dim);
}

template<class TMovingMesh>
bool
AccelerationRegularizer<TMovingMesh>::Evaluate(const double* const* parameters,
                                               double* residuals,
                                               double** jacobians) const
{

  const auto dim = TMovingMesh::PointType::Dimension;

  // Residuals
  auto p0 =
    this->initialPointsVector.at(this->GetPrevFrame())->ElementAt(this->index);
  for (unsigned int i = 0; i < dim; ++i)
    p0[i] += parameters[0][i];

  auto p1 = this->initialPointsVector.at(this->GetCurrentFrame())
              ->ElementAt(this->index);
  for (unsigned int i = 0; i < dim; ++i)
    p1[i] += parameters[1][i];

  auto p2 =
    this->initialPointsVector.at(this->GetNextFrame())->ElementAt(this->index);
  for (unsigned int i = 0; i < dim; ++i)
    p2[i] += parameters[2][i];

  this->movingVector.at(this->GetPrevFrame())->SetPoint(this->index, p0);
  this->movingVector.at(this->GetCurrentFrame())->SetPoint(this->index, p1);
  this->movingVector.at(this->GetNextFrame())->SetPoint(this->index, p2);

  for (unsigned int i = 0; i < dim; ++i) {
    residuals[i] = p2[i] - 2.0 * p1[i] + p0[i];
  }

  // Return if Jacobian wasn't requested.
  if (nullptr == jacobians) {
    return true;
  }

  // Set Jacobian to zero.
  for (unsigned int i = 0; i < 3; ++i) {
    if (nullptr != jacobians[i]) {
      ceres::MatrixRef(jacobians[i], dim, dim).setZero();
    }
  }

  // Set the relevant Jacobians.
  for (unsigned int i = 0; i < dim; ++i) {
    jacobians[0][i + i * dim] = 1;
    jacobians[1][i + i * dim] = -2;
    jacobians[2][i + i * dim] = 1;
  }

  return true;
}

template<class TMovingMesh>
unsigned int
AccelerationRegularizer<TMovingMesh>::GetNumberOfFrames() const
{
  return this->movingVector.size();
}

template<class TMovingMesh>
unsigned int
AccelerationRegularizer<TMovingMesh>::GetPrevFrame() const
{
  return vnl_math::remainder_floored((int(this->frame) - 1),
                                     int(this->GetNumberOfFrames()));
}

template<class TMovingMesh>
unsigned int
AccelerationRegularizer<TMovingMesh>::GetCurrentFrame() const
{
  return this->frame;
}

template<class TMovingMesh>
unsigned int
AccelerationRegularizer<TMovingMesh>::GetNextFrame() const
{
  return (this->frame + 1) % this->GetNumberOfFrames();
}

} // namespace sissr

#endif
