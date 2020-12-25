
#ifndef dvAccelerationRegularizer_h
#define dvAccelerationRegularizer_h

#include <ceres/ceres.h>
#include <limits>
#include <vnl/vnl_math.h>

namespace sissr {

template<class TMovingMesh>
class AccelerationRegularizer : public ceres::CostFunction
{

public:
  AccelerationRegularizer(
    const typename std::vector<typename TMovingMesh::Pointer>& _movingVector,
    const typename std::vector<typename TMovingMesh::PointsContainer::Pointer>&
      _initialPointsVector,
    unsigned int _frame,
    unsigned int _index);

  bool Evaluate(const double* const* parameters,
                double* residuals,
                double** jacobians) const;

  ~AccelerationRegularizer() {}

private:
  unsigned int GetNumberOfFrames() const;
  unsigned int GetPrevFrame() const;
  unsigned int GetCurrentFrame() const;
  unsigned int GetNextFrame() const;

  const typename std::vector<typename TMovingMesh::Pointer> movingVector;
  const typename std::vector<typename TMovingMesh::PointsContainer::Pointer>&
    initialPointsVector;

  const unsigned int frame;
  const unsigned int index;

}; // end class

} // namespace sissr

#include <dvAccelerationRegularizer.hxx>

#endif
