
#ifndef dvVelocityRegularizer_h
#define dvVelocityRegularizer_h

#include <ceres/ceres.h>
#include <limits>

namespace sissr {

template<class TMovingMesh>
class VelocityRegularizer : public ceres::CostFunction
{

public:
  VelocityRegularizer(
    const typename std::vector<typename TMovingMesh::Pointer>& _movingVector,
    const typename std::vector<typename TMovingMesh::PointsContainer::Pointer>&
      _initialPointsVector,
    unsigned int _frame,
    unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~VelocityRegularizer() {}

private:
  unsigned int GetNumberOfFrames() const;
  unsigned int GetCurrentFrame() const;
  unsigned int GetNextFrame() const;

  const typename std::vector<typename TMovingMesh::Pointer> movingVector;
  const typename std::vector<typename TMovingMesh::PointsContainer::Pointer>&
    initialPointsVector;

  const unsigned int frame;
  const unsigned int index;

}; // end class

} // namespace sissr

#include <dvVelocityRegularizer.hxx>

#endif
