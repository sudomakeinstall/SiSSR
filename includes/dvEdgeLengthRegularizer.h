
#ifndef dvEdgeLengthRegularizer_h
#define dvEdgeLengthRegularizer_h

#include <array>
#include <ceres/ceres.h>
#include <itkMacro.h>
#include <limits>

namespace sissr {

template<class TMesh>
class EdgeLengthRegularizer : public ceres::CostFunction
{

public:
  typedef typename TMesh::CoordRepType TReal;

  EdgeLengthRegularizer(
    const typename TMesh::Pointer& _moving,
    const typename TMesh::PointsContainer::Pointer& _initialPoints,
    unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~EdgeLengthRegularizer() {}

private:
  const typename TMesh::Pointer& moving;
  const typename TMesh::PointsContainer::Pointer& initialPoints;

  unsigned int index;
  std::array<unsigned int, 2> point_indices;

}; // end class

} // namespace sissr

#include <dvEdgeLengthRegularizer.hxx>

#endif
