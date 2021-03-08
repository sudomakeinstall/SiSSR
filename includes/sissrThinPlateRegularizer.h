#ifndef sissr_ThinPlateRegularizer_h
#define sissr_ThinPlateRegularizer_h

// STD
#include <limits>

// ITK
#include <itkMacro.h>

// Ceres
#include <ceres/ceres.h>

// VNL
#include <vnl_matrix_fixed.h>

namespace sissr {

template<class TMesh>
class ThinPlateRegularizer :
public ceres::CostFunction
{

  public:

  using TReal = typename TMesh::CoordRepType;

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

#include <sissrThinPlateRegularizer.hxx>

#endif
