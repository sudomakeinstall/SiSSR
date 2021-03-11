#ifndef itk_LoopSubdivisionSurfaceMesh_h
#define itk_LoopSubdivisionSurfaceMesh_h

// std
#include <utility>
#include <math.h>

// ITK
#include "itkQuadEdgeMesh.h"
#include "itkConceptChecking.h"
#include "itkLoopSubdivisionSurfaceMatrices.h"

// VXL
#include "vnl/vnl_power.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"

namespace itk
{
/**
 * \class LoopSubdivisionSurfaceMesh
 * \brief Representation of a Loop subdivision surface.
 *
 * "Extended" Subdivision Matrix (A):
 * "subdivides the extraordinary component" (Stebbing, 73).
 *
 * | S   0   |
 * | S11 S12 |
 *
 * "Bigger" Subdivision Matrix (B):
 * "subdivides the extraordinary component and generates the
 * patch vertices which define the three 'uncovered' ordinary
 * triangle B-spline patches" (Stebing, 74).
 *
 * | S   0   |
 * | S11 S12 |
 * | S21 S22 |
 * 
 * Possible Optimizations:
 * - Precalculate matrices for the desired Ns
 * - Use Eigen system
 * - NB: These optimizations are probably unnecessary unless
 *       the mesh is very large/has highly extraordinary points.
 *
 * \author Davis Vigneault
 *
 * \ingroup ITKDVUtilities
 */
template< typename TReal, unsigned int VDimension,
          typename TTraits = QuadEdgeMeshTraits< TReal, VDimension, bool, bool, TReal, TReal > >
class LoopSubdivisionSurfaceMesh:
public QuadEdgeMesh< TReal, VDimension, TTraits >
{
public:

  /** Standard typedefs. */
  using Self = LoopSubdivisionSurfaceMesh;
  using Superclass = QuadEdgeMesh<TReal, VDimension, TTraits>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using RealType = TReal;

  // Points
  using PointIdentifier = typename Superclass::PointIdentifier;
  using PointType = typename Superclass::PointType;
  using PointsContainer = typename Superclass::PointsContainer;
  using PointsContainerConstIterator = typename Superclass::PointsContainerConstIterator;
  using PointsContainerIterator = typename Superclass::PointsContainerIterator;

  // Cells
  using CellIdentifier = typename Superclass::CellIdentifier;
  using CellType = typename Superclass::CellType;
  using CellAutoPointer = typename Superclass::CellAutoPointer;
  using CellsContainer = typename Superclass::CellsContainer;
  using CellsContainerConstIterator = typename Superclass::CellsContainerConstIterator;
  using CellsContainerIterator = typename Superclass::CellsContainerIterator;

  // Edges
  using QEPrimal = typename Superclass::QEPrimal;

  /** Real type, minimum N, maximum N. */
  using TMatrices = LoopSubdivisionSurfaceMatrices<TReal, 3, 25>;
  using TParameters = typename TMatrices::TParameters;
  using TNMap = std::map<CellIdentifier, unsigned int>;
  using TValencyMap = std::map<PointIdentifier, unsigned int>;
  using TSurfaceParameter = std::pair<CellIdentifier, TParameters>;
  using TSurfaceParameterList = std::vector<TSurfaceParameter>;
  using TOneRingMap = std::map<CellIdentifier, vnl_vector<PointIdentifier>>;

  /** Basic Object interface. */
  itkNewMacro(Self);
  itkTypeMacro(LoopSubdivisionSurfaceMesh, QuadEdgeMesh);

  /** Calculates the valency map, the N map, and the surface parameter list. */
  void Setup();
  PointType GetPointOnSurface(const CellIdentifier &cellID, const TParameters &p) const;
  itkGetConstMacro(SurfaceParameterList, TSurfaceParameterList);

  vnl_vector<TReal> GetResidualBlock(const CellIdentifier &cellID, const TParameters &p) const;

  // Utility functions
  vnl_vector<PointIdentifier> GetPointListForCell(const CellIdentifier &cellID) const
    {
    return this->m_OneRingMap.at(cellID);
    }

  TSurfaceParameter
  GetSurfaceParameter(const size_t &i)
    {
    return this->m_SurfaceParameterList.at(i);
    }

  // Matrices
  vnl_matrix<TReal> CalculateControlPointMatrix(const CellIdentifier &cellID) const;
  vnl_vector_fixed<TReal,45> CalculateThinPlateEnergyVectorForCell(const CellIdentifier &cellID) const;
  static const TMatrices m_Matrices;
  unsigned int GetNForCell(const CellIdentifier &cellID) const
    { return this->m_NMap.at(cellID); }

  ////////////////
  // Properties //
  ////////////////

  TReal CalculateSurfaceArea() const;
  TReal CalculateSurfaceAreaForCell(const CellIdentifier &cellID) const;

  itkSetMacro( SurfaceSampleDensity, unsigned int );
  itkGetConstMacro( SurfaceSampleDensity, unsigned int );

protected:

  /** Constructor and Destructor. */
  LoopSubdivisionSurfaceMesh();
  ~LoopSubdivisionSurfaceMesh(){};

  /** Called by Setup() method. */
  TNMap                 CalculateNMap();
  TValencyMap           CalculateValencyMap();
  TSurfaceParameterList CalculateParameterList();
  vnl_vector<PointIdentifier> CalculatePointListForCell(const CellIdentifier &cellID) const;
  TOneRingMap           CalculateOneRingMap();

  /** Called by CalculateNMap(). */
  unsigned int CalculateNForCellID(const CellIdentifier &cellID) const;

  /** Assigned to by Setup() method. */
  TNMap                 m_NMap;
  TValencyMap           m_VMap;
  TOneRingMap           m_OneRingMap;
  TSurfaceParameterList m_SurfaceParameterList;
  unsigned int          m_SurfaceSampleDensity = 2;

private:

  ITK_DISALLOW_COPY_AND_ASSIGN(LoopSubdivisionSurfaceMesh);

};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLoopSubdivisionSurfaceMesh.hxx"
#endif

#endif
