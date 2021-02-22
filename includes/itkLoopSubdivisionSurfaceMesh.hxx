#ifndef itk_LoopSubdivisionSurfaceMesh_hxx
#define itk_LoopSubdivisionSurfaceMesh_hxx

// VNL
#include "vnl/algo/vnl_matrix_inverse.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"

// ITK
#include "itkLoopSubdivisionSurfaceMesh.h"
#include "itkTriangleHelper.h"
#include "itkMacro.h"

namespace itk
{

///////////////////
// Static member //
///////////////////

template< typename TReal, unsigned int VDimension, typename TTraits >
const typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::TMatrices
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::m_Matrices = TMatrices();

/////////////////
// Constructor //
/////////////////

template< typename TReal, unsigned int VDimension, typename TTraits >
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::LoopSubdivisionSurfaceMesh()
{}

/////////////////////////////////
// Setup and utility functions //
/////////////////////////////////

template< typename TReal, unsigned int VDimension, typename TTraits >
void
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::Setup()
{
  this->m_VMap                 = this->CalculateValencyMap();
  this->m_NMap                 = this->CalculateNMap();
  this->m_SurfaceParameterList = this->CalculateParameterList();
  this->m_OneRingMap           = this->CalculateOneRingMap();
}

template< typename TReal, unsigned int VDimension, typename TTraits >
typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::TValencyMap
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateValencyMap()
{
  TValencyMap valencyMap;
  for (auto it = this->GetPoints()->Begin();
       it != this->GetPoints()->End();
       ++it)
    valencyMap.emplace(it->Index(), this->GetPoint(it->Index()).GetValence());
  return valencyMap;
}

template< typename TReal, unsigned int VDimension, typename TTraits >
typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::TOneRingMap
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateOneRingMap()
{
  TOneRingMap local_Map;
  for (auto it = this->GetCells()->Begin();
       it != this->GetCells()->End();
       ++it)
    {
    local_Map[it.Index()] = this->CalculatePointListForCell(it.Index());
    }
  return local_Map;
}

template< typename TReal, unsigned int VDimension, typename TTraits >
typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::TNMap
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateNMap()
{

  TNMap local_m_NMap;
  for (auto it = this->GetCells()->Begin();
       it != this->GetCells()->End();
       ++it)
    {

    local_m_NMap[it.Index()] = this->CalculateNForCellID(it.Index());

    if (local_m_NMap[it.Index()] > TMatrices::MaximumValency)
      {
      std::string e  = "ERROR: Maximum valency has been exceeded.\n";
                  e += "\tValency: " + std::to_string(local_m_NMap[it.Index()]);
      itkAssertOrThrowMacro(false, e);
      }

    if (local_m_NMap[it.Index()] < TMatrices::MinimumValency)
      {
      std::string e  = "ERROR: Minimum valency has been exceeded.\n";
                  e += "\tValency: " + std::to_string(local_m_NMap[it.Index()]);
      itkAssertOrThrowMacro(false, e);
      }

    }
  return local_m_NMap;

}

template<typename TReal, unsigned int VDimension, typename TTraits>
typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::TSurfaceParameterList
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateParameterList()
{
  TSurfaceParameterList surfaceParameters;

  itkAssertOrThrowMacro( this->m_SurfaceSampleDensity > 0,
                         "Surface sample density must be greater than zero." );

  TReal StepSize = 1.0 / (TReal(this->m_SurfaceSampleDensity) + 1.0);

  for (auto it  = this->GetCells()->Begin();
            it != this->GetCells()->End();
          ++it)
    {
    for (TReal s = StepSize/2; s <= 1.0; s += StepSize)
      {
      for (TReal t = StepSize/2; t <= 1.0-s; t += StepSize)
        {
        const auto p = std::make_pair(s, t);
        const auto u = std::make_pair(it.Index(), p);
        surfaceParameters.push_back(u);
        }
      }
    }

  return surfaceParameters;
}

//
//
//

template< typename TReal, unsigned int VDimension, typename TTraits >
typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::PointType
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::GetPointOnSurface(const CellIdentifier &cellID, const TParameters &p) const
{

  const auto C0 = this->CalculateControlPointMatrix(cellID);
  vnl_matrix<TReal> C0_copy(C0.data_block(), C0.rows(), C0.cols());

  const auto R = this->GetResidualBlock(cellID,p);
  const auto pointValues(C0_copy.transpose()*R);
  const PointType point(pointValues.data_block());

  return point;

}

template< typename TReal, unsigned int VDimension, typename TTraits >
vnl_vector< TReal >
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::GetResidualBlock(const CellIdentifier &cellID, const TParameters &p) const
{

  itkAssertOrThrowMacro(this->m_Matrices.VerifyParameters(p),
                        "The parameters provided are invalid.");

  const auto N = this->m_NMap.at(cellID);

  if (6 == N)
    {
    return this->m_Matrices.CalculateBarycentricBSplineWeights(p).as_vector();
    }

  if ( std::abs( std::get<0>(p) ) > 10e-6 || std::abs( std::get<1>(p) ) > 10e-6 )
    {
    const auto p_t = this->m_Matrices.TransformParametersToPatch(p);
    itkAssertOrThrowMacro(this->m_Matrices.VerifyParameters(p_t),
                        "The transformed parameters are invalid.");
    const auto A = this->m_Matrices.GetA(N);
    const auto B = this->m_Matrices.GetB(N);
    vnl_matrix<TReal> B_copy(B.data_block(), B.rows(), B.cols());
    const unsigned int k = this->m_Matrices.CalculateChildIndex(p);
    const unsigned int l = this->m_Matrices.CalculateNumberOfRequiredSubdivisions(p);
    const auto wts = this->m_Matrices.CalculateBarycentricBSplineWeights(p_t);
    const auto P = this->m_Matrices.GetP(N,k);
    vnl_matrix<TReal> P_copy(P.data_block(), P.rows(), P.cols());
  
    return (P_copy*B_copy*vnl_power(A,l-1)).transpose()*wts;
    }
  else // Deal with origin separately
    {
    const auto es = this->m_Matrices.GetSortedEigensystem(N);

    const auto sortedRightEigenvectors = std::get<1>(es);
    const auto sortedLeftEigenvectors = std::get<2>(es);

    const auto v1R = sortedRightEigenvectors.get_column(0);
    itkAssertOrThrowMacro(((v1R.sum() / v1R.size()) - v1R[0]) < 10e-6, "");
    return sortedLeftEigenvectors.get_column(0) * v1R.get(0);
    }

}

template< typename TReal, unsigned int VDimension, typename TTraits >
unsigned int
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateNForCellID(const CellIdentifier &cellID) const
{

  // Get a pointer to the cell
  CellAutoPointer cell;

  // Ensure a cell was found
  itkAssertOrThrowMacro( this->GetCell(cellID, cell), "No cell was returned.");

  // Ensure the cell is a triangle
  itkAssertOrThrowMacro( (3 == cell->GetNumberOfPoints()), "The cell must have three vertices.");

  // Ensure that the number of extraordinary vertices is <= 1 
  unsigned int numberOfExtraordinaryVertices = 0;
  PointIdentifier extraordinaryID;  
  for (auto it = cell->PointIdsBegin();
       it != cell->PointIdsEnd();
       ++it)
    {
    if (6 != this->m_VMap.at(*it))
      {
      ++numberOfExtraordinaryVertices;
      extraordinaryID = *it;
      }
    }

  switch (numberOfExtraordinaryVertices)
    {
    case 0:
      return 6;
      break;
    case 1:
      return this->m_VMap.at(extraordinaryID);
      break;
    default:
      const auto pid0 = cell->PointIdsBegin()[0];
      const auto pid1 = cell->PointIdsBegin()[1];
      const auto pid2 = cell->PointIdsBegin()[2];
      std::cerr << "Too many extraordinary vertices: " << numberOfExtraordinaryVertices << '\n'
        << "    Cell Identifier: " << cellID << '\n'
        << "    Point ID 0: " << pid0 << "(" << this->GetPoint(pid0) << "\n)"
        << "    Point ID 1: " << pid1 << "(" << this->GetPoint(pid1) << "\n)"
        << "    Point ID 2: " << pid2 << "(" << this->GetPoint(pid2) << "\n)";
      itkExceptionMacro("Too many extraordinary vertices.");
      break;
    }

}

template< typename TReal, unsigned int VDimension, typename TTraits >
vnl_vector<typename LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>::PointIdentifier>
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculatePointListForCell(const CellIdentifier &cellID) const
{

  const auto N = this->GetNForCell(cellID);

  // Get a pointer to the cell
  CellAutoPointer cell;
  // Ensure a cell was found
  itkAssertOrThrowMacro( (this->GetCell(cellID, cell)) , "No cell was returned.");

  // Ensure the cell is a triangle
  itkAssertOrThrowMacro( (3 == cell->GetNumberOfPoints()), "The cell must have three vertices.");

  // Ensure that the number of extraordinary vertices is <= 1 
  unsigned int numberOfExtraordinaryVertices = 0;
  for (auto it = cell->PointIdsBegin(); it != cell->PointIdsEnd(); ++it)
    if (6 != this->m_VMap.at(*it))
      ++numberOfExtraordinaryVertices;
  itkAssertOrThrowMacro((2 > numberOfExtraordinaryVertices), "Too many extraordinary vertices.");
  
  // Assign pOrigin to the extraordinary vertex if one exists
  // and to an arbitrary vertex otherwise
  auto it = cell->PointIdsBegin();
  auto pOrigin = *(it);
  if (6 != this->m_VMap.at(*(it+1))) pOrigin = *(it+1);
  if (6 != this->m_VMap.at(*(it+2))) pOrigin = *(it+2);
 
  QEPrimal* edge = this->FindEdge(pOrigin);
  itkAssertOrThrowMacro((nullptr != edge), "Edge not found (origin circle).");

  while (cellID != edge->GetLeft())
    edge = edge->GetOnext();

  // Calculate point list
  vnl_vector<PointIdentifier> pointList(N+6);
  unsigned int pointId = 0;
  pointList[pointId++] = edge->GetOrigin();

  QEPrimal* temp = edge;
  do
    {
    temp = temp->GetOnext();
    pointList[pointId++] = temp->GetDestination();
    } while (temp != edge);

  // Left circle
  edge = this->FindEdge(pointList[1], pointList[N]);
  itkAssertOrThrowMacro((nullptr != edge), "Edge not found (left circle).");
  edge = edge->GetOnext(); // Segfault on 24th iteration
  pointList[pointId++] = edge->GetDestination();
  edge = edge->GetOnext();
  pointList[pointId++] = edge->GetDestination();
  edge = edge->GetOnext();
  pointList[pointId++] = edge->GetDestination();

  // Right circle
  edge = this->FindEdge(pointList[N], pointList[1]);
  itkAssertOrThrowMacro((nullptr != edge), "Edge not found (right circle).");
  edge = edge->GetOprev()->GetOprev();
  pointList[pointId++] = edge->GetDestination();
  edge = edge->GetOprev();
  pointList[pointId++] = edge->GetDestination();
  return pointList;

}

template< typename TReal, unsigned int VDimension, typename TTraits >
vnl_matrix<TReal>
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateControlPointMatrix(const CellIdentifier &cellID) const
{
  const auto N = this->GetNForCell(cellID);
  vnl_matrix<TReal> C0(N+6,3);
//  const auto L = this->CalculatePointListForCell(cellID);
  const auto L = this->GetPointListForCell(cellID);
  for (unsigned int r = 0; r < (N+6); ++r)
    C0.set_row(r,this->GetPoint(L[r]).GetVnlVector());
  return C0;
}

template< typename TReal, unsigned int VDimension, typename TTraits >
vnl_vector_fixed<TReal,45>
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateThinPlateEnergyVectorForCell(const CellIdentifier &cellID) const
{

  const auto X = this->CalculateControlPointMatrix(cellID);
  const vnl_matrix<TReal> X_copy(X.data_block(), X.rows(), X.cols());

  const auto N = this->GetNForCell(cellID);
  const auto P = this->m_Matrices.GetBSplineToBezier(N);
  const vnl_matrix<TReal> P_copy(P.data_block(), P.rows(), P.cols());

  const auto B = (P_copy * X_copy).flatten_column_major();
  const auto M = this->m_Matrices.GetBezierMRootBlock();
  const vnl_matrix<TReal> M_copy(M.data_block(), M.rows(), M.cols());

  return M_copy * B;
  
}

template< typename TReal, unsigned int VDimension, typename TTraits >
TReal
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateSurfaceArea() const
{
  TReal volume = 0.;
  for (auto it = this->GetCells()->Begin();
       it != this->GetCells()->End();
       ++it)
    {
    volume += this->CalculateSurfaceAreaForCell(it.Index());
    }

  return volume;
}

template< typename TReal, unsigned int VDimension, typename TTraits >
TReal
LoopSubdivisionSurfaceMesh<TReal,VDimension,TTraits>
::CalculateSurfaceAreaForCell(const CellIdentifier &cellID) const
{

  /*
   * 1. Get the cell's control points.
   * 2. Create a mesh.
   * 3. Subdivide the specified number of times.
   * 4. Sum the areas of all internal triangles.
   */

  TReal area = 0.0;

  TReal StepSize = 0.04;
  for (TReal s = 0.0; (s + StepSize) < 1.0; s += StepSize)
    {
    for (TReal t = 0.0; (t + StepSize) < (1.0 - s); t += StepSize)
      {

      const auto p_a = std::make_pair(s           , t           );
      const auto p_b = std::make_pair(s           , t + StepSize);
      const auto p_c = std::make_pair(s + StepSize, t           );

      const auto a = this->GetPointOnSurface(cellID, p_a);
      const auto b = this->GetPointOnSurface(cellID, p_b);
      const auto c = this->GetPointOnSurface(cellID, p_c);

      area += itk::TriangleHelper<Self::PointType>::ComputeArea(a, b, c);

      if (( s + t + 2. * StepSize) < 1.0)
        {
        const auto p_d = std::make_pair(s + StepSize, t + StepSize);
        const auto d = this->GetPointOnSurface(cellID, p_d);
        area += itk::TriangleHelper<Self::PointType>::ComputeArea(d, b, c);
        }
      }
    }

  return area;

}

} // namespace itk

#endif
