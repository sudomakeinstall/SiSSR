#ifndef sissr_TriangleAspectRatioRegularizer_h
#define sissr_TriangleAspectRatioRegularizer_h

// STD
#include <array>
#include <limits>

// ITK
#include <itkMacro.h>

// Ceres
#include <ceres/ceres.h>

namespace sissr {

template<class TMesh>
class TriangleAspectRatioRegularizer : public ceres::CostFunction
{

public:
  using TReal = typename TMesh::CoordRepType;

  TriangleAspectRatioRegularizer(
    const typename TMesh::Pointer& _moving,
    const typename TMesh::PointsContainer::Pointer& _initialPoints,
    unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~TriangleAspectRatioRegularizer() {}

private:
  const typename TMesh::Pointer& moving;
  const typename TMesh::PointsContainer::Pointer& initialPoints;

  unsigned int index;
  std::array<unsigned int, 3> point_indices;

}; // end class

} // namespace sissr

#include <sissrTriangleAspectRatioRegularizer.hxx>

#endif
