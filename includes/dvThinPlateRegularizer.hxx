
#ifndef dvThinPlateRegularizer_hxx
#define dvThinPlateRegularizer_hxx

#include <dvThinPlateRegularizer.h>

#include <limits>
#include <ceres/ceres.h>
#include <itkMacro.h>

namespace dv
{

template<class TMesh>
ThinPlateRegularizer<TMesh>
::ThinPlateRegularizer(
  const typename TMesh::Pointer
    &_moving,
  const typename TMesh::PointsContainer::Pointer
    &_initialPoints,
  unsigned int _index) :
    moving(_moving),
    initialPoints(_initialPoints),
    index(_index)
{

  // The *parameters* in this case are the point positions.
  // That is, the TP energy is dictated by the point positions.
  const auto cellList = this->moving->GetPointListForCell(this->index);

  for (std::size_t i = 0; i < cellList.size(); ++i)
    {
    // Each point has an x, y, and z coordinate
    this->mutable_parameter_block_sizes()->push_back(3);
    }

  // Each instance of this class is responsible for one cell.
  // We use the Bezier representation regardless of whether
  // the cell is ordinary or extraordinary.  Therefore,
  // each cell corresponds to 45 residuals.
  this->set_num_residuals(45);
}

template<class TMesh>
bool
ThinPlateRegularizer<TMesh>
::Evaluate(const double* const* parameters,
           double* residuals,
           double** jacobians) const
{

  //////////////////////////////////////////////////
  // Ensure that the point positions are updated. //
  //////////////////////////////////////////////////

  const auto L =
    this->moving->GetPointListForCell(index);

  for (std::size_t i = 0; i < L.size(); ++i)
    {
    const auto init = this->initialPoints->ElementAt( L[i] );
    const auto diff = parameters[i];
    typename TMesh::PointType current;
    for (unsigned int d = 0; d < 3; ++d)
      {
      current[d] = init[d] + diff[d];
      }
    this->moving->SetPoint( L[i], current );
    }

  ///////////////
  // Residuals //
  ///////////////

  const auto residualVector
    = this->moving->CalculateThinPlateEnergyVectorForCell(this->index);
  // Residuals are organized COLUMNWISE.
  // [xxxxxxxxxxxxxxxyyyyyyyyyyyyyyyzzzzzzzzzzzzzzz]
  // Keep this in mind while calculating the Jacobian, below.
  for (unsigned int r = 0; r < 45; ++r)
    {
    residuals[r] = residualVector.get(r);
    }

  // Return if Jacobian wasn't requested.
  if (nullptr == jacobians)
    {
    return true;
    }

  // Set Jacobian to zero.

  const unsigned int N // valency of cell
    = this->moving->GetNForCell(this->index);
  const auto M
    = this->moving->m_Matrices.GetBezierMRoot();
  const auto P
    = this->moving->m_Matrices.GetBSplineToBezier(N);
  const vnl_matrix_fixed<TReal, 15, 15> M_copy(M.data_block());
  const auto B = M * P;

  for (std::size_t i = 0; i < L.size(); ++i)
    {
    if (nullptr == jacobians[i]) continue;
    // Jacobian isn an M x N matrix (M rows, N columns)
    // M: Number of residuals (45)
    // N: Number of parameters (3)
    // In our case, every point has a jacobian
    // Every point has three dimensions
    // Each dimension affects 15 residuals
    // The effect on the remaining 30 is zero.
    ceres::MatrixRef(jacobians[i],45,3).setZero();

    // Each point has three dimensions, so loop over that.
    for (unsigned int d = 0; d < 3; ++d)
      {

      // Each dimension of each point affects 15 residuals.
      // The effect on the remaining 30 is zero.
      // Loop over that as well.
      for (unsigned int r = 0; r < 15; ++r)
        {
        const unsigned int row = 15*d+r;
        const unsigned int col = d;
        const unsigned int stride = 3;
        const unsigned int offset = row*stride+col;
        jacobians[i][offset] = B.get(r,i);
        }
      }
    }

  return true;

}

} // end namespace

#endif

