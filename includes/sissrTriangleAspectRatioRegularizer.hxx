#ifndef sissr_TriangleAspectRatioRegularizer_hxx
#define sissr_TriangleAspectRatioRegularizer_hxx

#include <sissrTriangleAspectRatioRegularizer.h>

#include <ceres/ceres.h>
#include <itkMacro.h>
#include <limits>

namespace sissr {

template<class TMesh>
TriangleAspectRatioRegularizer<TMesh>::TriangleAspectRatioRegularizer(
  const typename TMesh::Pointer& _moving,
  const typename TMesh::PointsContainer::Pointer& _initialPoints,
  unsigned int _index)
  : moving(_moving)
  , initialPoints(_initialPoints)
  , index(_index)
{
  typename TMesh::CellAutoPointer cell;
  this->moving->GetCell(this->index, cell);

  for (unsigned int i = 0; i < 3; ++i) {
    this->point_indices[i] = *(cell->PointIdsBegin() + i);
    this->mutable_parameter_block_sizes()->push_back(
      TMesh::PointType::Dimension);
  }

  this->set_num_residuals(1);
}

template<class TMesh>
bool
TriangleAspectRatioRegularizer<TMesh>::Evaluate(const double* const* parameters,
                                                double* residuals,
                                                double** jacobians) const
{

  //////////////////////////////////////////////////
  // Ensure that the point positions are updated. //
  //////////////////////////////////////////////////

  for (unsigned int i = 0; i < this->point_indices.size(); ++i) {
    const auto init = this->initialPoints->ElementAt(point_indices[i]);
    typename TMesh::PointType current;
    for (unsigned int d = 0; d < TMesh::PointType::Dimension; ++d) {
      current[d] = init[d] + parameters[i][d];
    }
    this->moving->SetPoint(this->point_indices[i], current);
  }

  /////////////////////////////////////
  // Find largest and smallest edges //
  /////////////////////////////////////

  std::array<typename TMesh::PointType, 3> points;
  points[0] = this->moving->GetPoint(this->point_indices[0]);
  points[1] = this->moving->GetPoint(this->point_indices[1]);
  points[2] = this->moving->GetPoint(this->point_indices[2]);

  std::array<double, 3> lengths;
  lengths[0] = points[0].EuclideanDistanceTo(points[1]);
  lengths[1] = points[1].EuclideanDistanceTo(points[2]);
  lengths[2] = points[2].EuclideanDistanceTo(points[0]);

  int minimum = 0;
  if (lengths[1] < lengths[0])
    minimum = 1;
  if (lengths[2] < lengths[1])
    minimum = 2;

  int maximum = 0;
  if (lengths[1] > lengths[0])
    maximum = 1;
  if (lengths[2] > lengths[1])
    maximum = 2;

  assert(minimum != maximum);

  int other = 0;
  if ((other == minimum) || (other == maximum))
    other = 1;
  if ((other == minimum) || (other == maximum))
    other = 2;

  assert(minimum != other);
  assert(maximum != other);

  residuals[0] = lengths[maximum] / lengths[minimum] - 1.0;

  // Continue if not requested
  if (nullptr == jacobians)
    return true;

  // Jacobian is an M x N matrix (M rows, N columns)
  // M: Number of resuduals (2; length ratios)
  // N: Number of prameters in block (3, point coordinates)

  for (unsigned int d = 0; d < TMesh::PointType::Dimension; ++d) {

    if (nullptr != jacobians[maximum]) {
      const auto Lmax_prime =
        -(points[(maximum + 1) % 3][d] - points[maximum][d]) / lengths[maximum];
      const auto Lmin_prime =
        (minimum == ((maximum + 1) % 3)
           ? 0.0
           : (points[maximum][d] - points[minimum][d]) / lengths[minimum]);
      jacobians[maximum][d] =
        ((lengths[minimum] * Lmax_prime) - (lengths[maximum] * Lmin_prime)) /
        (lengths[minimum] * lengths[minimum]);
    }

    if (nullptr != jacobians[minimum]) {
      const auto Lmin_prime =
        -(points[(minimum + 1) % 3][d] - points[minimum][d]) / lengths[minimum];
      const auto Lmax_prime =
        (maximum == (minimum + 1) % 3)
          ? 0.0
          : (points[minimum][d] - points[maximum][d]) / lengths[maximum];
      jacobians[minimum][d] =
        ((lengths[minimum] * Lmax_prime) - (lengths[maximum] * Lmin_prime)) /
        (lengths[minimum] * lengths[minimum]);
    }

    if (nullptr != jacobians[other]) {
      const auto Lmax_prime =
        (other == ((maximum + 1) % 3))
          ? (points[other][d] - points[maximum][d]) / lengths[maximum]
          : 0.0;
      const auto Lmin_prime =
        (other == ((minimum + 1) % 3))
          ? (points[other][d] - points[minimum][d]) / lengths[minimum]
          : 0.0;
      jacobians[other][d] =
        ((lengths[minimum] * Lmax_prime) - (lengths[maximum] * Lmin_prime)) /
        (lengths[minimum] * lengths[minimum]);
    }
  }

  return true;
}

} // namespace sissr

#endif
