
#ifndef dvThinPlateRegularizer_h
#define dvThinPlateRegularizer_h

#include <limits>
#include <ceres/ceres.h>
#include <itkMacro.h>
#include <vnl_matrix_fixed.h>

namespace sissr {

template<class TMesh>
class ThinPlateRegularizer :
public ceres::CostFunction
{

  public:

  typedef typename TMesh::CoordRepType TReal;

  ThinPlateRegularizer(
    const typename TMesh::Pointer
      &_moving,
    const typename TMesh::PointsContainer::Pointer
      &_initialPoints,
    unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~ThinPlateRegularizer(){}

private:

  const typename TMesh::Pointer &moving;
  const typename TMesh::PointsContainer::Pointer &initialPoints;

  unsigned int index;

}; // end class

} // namespace sissr

#include <dvThinPlateRegularizer.hxx>

#endif

