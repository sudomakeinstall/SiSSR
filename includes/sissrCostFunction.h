#ifndef sissr_CostFunction_h
#define sissr_CostFunction_h

// STD
#include <limits>

// ITK
#include <itkPointsLocator.h>

// Ceres
#include <ceres/ceres.h>

namespace sissr {

template<class TFixedMesh, class TMovingMesh>
class CostFunction :
public ceres::CostFunction
{

  public:

  typedef typename TFixedMesh::PointsContainer TContainer;
// TODO
//  typedef itk::PointsLocator< TContainer > TLocator;
  typedef typename std::map<unsigned char, typename itk::PointsLocator< TContainer >::Pointer> TLocatorMap;

  CostFunction(
// TODO
//               const typename TLocator::Pointer &_locator,
               const TLocatorMap &_locator,
               const typename TMovingMesh::Pointer &_moving,
               const typename TMovingMesh::PointsContainer::Pointer &_initialPoints,
               unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~CostFunction(){}

private:

// TODO
//  const typename TLocator::Pointer &locator;
  const TLocatorMap &locator;
  const typename TMovingMesh::Pointer &moving;
  const typename TMovingMesh::PointsContainer::Pointer &initialPoints;

  const unsigned int index;
  const typename TMovingMesh::TSurfaceParameter param;
  const vnl_vector<typename TMovingMesh::RealType> residual;
  const vnl_vector<typename TMovingMesh::PointIdentifier> L;

}; // end class

} // namespace sissr

#include <sissrCostFunction.hxx>

#endif
