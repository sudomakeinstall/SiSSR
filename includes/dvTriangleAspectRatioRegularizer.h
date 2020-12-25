
#ifndef dvTriangleAspectRatioRegularizer_h
#define dvTriangleAspectRatioRegularizer_h

#include <array>
#include <ceres/ceres.h>
#include <itkMacro.h>
#include <limits>

namespace sissr {

template<class TMesh>
class TriangleAspectRatioRegularizer : public ceres::CostFunction
{

public:
  typedef typename TMesh::CoordRepType TReal;

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

#include <dvTriangleAspectRatioRegularizer.hxx>

#endif
