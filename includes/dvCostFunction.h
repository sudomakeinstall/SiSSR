#ifndef dvCostFunction_h
#define dvCostFunction_h

#include <limits>
#include <ceres/ceres.h>
#include <itkPointsLocator.h>

namespace dv
{
template<class TFixedMesh, class TMovingMesh>
class CostFunction :
public ceres::CostFunction
{

  public:

  typedef typename TFixedMesh::PointsContainer TContainer;
  typedef itk::PointsLocator< TContainer > TLocator;

  CostFunction(
               const typename TLocator::Pointer
                 &_locator,
               const typename TMovingMesh::Pointer
                 &_moving,
               const typename TMovingMesh::PointsContainer::Pointer
                 &_initialPoints,
               unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~CostFunction(){}

private:

//  unsigned int GetNumberOfControlPoints() const;
//  unsigned int GetNumberOfSurfacePoints() const;

  const typename TLocator::Pointer &locator;
  const typename TMovingMesh::Pointer &moving;
  const typename TMovingMesh::PointsContainer::Pointer &initialPoints;

  const unsigned int index;
  const typename TMovingMesh::TSurfaceParameter param;
  const vnl_vector<typename TMovingMesh::RealType> residual;
  const vnl_vector<typename TMovingMesh::PointIdentifier> L;

}; // end class
} // end namespace

#include <dvCostFunction.hxx>

#endif

