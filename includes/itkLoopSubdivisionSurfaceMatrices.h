#ifndef itk_LoopSubdivisionSurfaceMatrices_h
#define itk_LoopSubdivisionSurfaceMatrices_h

// STD
#include <complex>
#include <map>
#include <array>

// VNL
#include "vnl/vnl_matrix.hxx"
#include "vnl/vnl_matrix_fixed.hxx"
#include "vnl/vnl_vector.hxx"
#include "vnl/vnl_vector_fixed.hxx"

namespace itk
{
/**
 * \class LoopSubdivisionSurfaceMatrices
 * \brief Matrices used for evaluation of Loop subdivision surfaces at arbitrary positions.
 *
 * Quadratic triangle B-splines generalize uniform quadratic B-splines
 * to *regular* triangle meshes--i.e., a mesh where all patches are
 * triangles, and all triangles are "ordinary" (meaning that every
 * vertex is of valency six).
 *
 * Loop subdivision surfaces in turn generalize quadratic triangle
 * B-splines to *irregular* triangle meshes, without restricting
 * the valency of the vertices.  Loop described an algorithm in
 * which a triangle mesh could be progressively refined towards a
 * smooth limit surface [1].  Stam later demonstrated that a point
 * on the surface could be evaluated exactly at arbitrary parameter
 * values for a patch with one extraordinary vertex (which can be
 * achieved in practice by undergoing one round of Loop subdivision)
 * [2].  This class implements the matrices necessary for this
 * evaluation, as well as for related concepts such as calculation
 * of thin-plate energy and Bezier-patch approximation.
 *
 * [1] Smooth Subdivision Surfaces Based on Triangles.  Charles Loop.
 * [2] Evaluation of Loop Subdivision Surfaces.  Jos Stam.
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
 * \author Davis Vigneault
 *
 * \ingroup ITKDVUtilities
 *
 */
template< typename TReal, unsigned int MinN = 4, unsigned int MaxN = 25 >
class LoopSubdivisionSurfaceMatrices
{

static_assert(MaxN >= MinN,
              "The third template argument must be >= the second.");
static_assert(MinN >= 3,
              "The minimum N may not be less than 4.");
static_assert(MaxN >= 6,
              "The maximum N may not be less than 6.");

  typedef LoopSubdivisionSurfaceMatrices   Self;
  typedef std::array<vnl_matrix<TReal>, 3> TPickers;

public:

  /* Constructor
   * The matrices from MinN <= N <= MaxN are calculated and stored in a map.
   * Getters are then supplied.
   */
  LoopSubdivisionSurfaceMatrices();

  static constexpr unsigned int MinimumValency = MinN;
  static constexpr unsigned int MaximumValency = MaxN;

  typedef std::pair< TReal, TReal > TParameters;
  typedef std::complex< TReal >     TComplex;

  /////////////////////
  // Access Matrices //
  /////////////////////

  /* Get subdivision matrix. */
  const vnl_matrix<TReal>& GetS(const unsigned int &n) const
    { return this->m_S_map.at(n); }
  /* Get extended subdivision matrix. */
  const vnl_matrix<TReal>& GetA(const unsigned int &n) const
    { return this->m_A_map.at(n); }
  /* Get bigger subdivision matrix. */
  const vnl_matrix<TReal>& GetB(const unsigned int &n) const
    { return this->m_B_map.at(n); }
  const vnl_matrix<TReal>& GetP(const unsigned int &n,
                                const unsigned int &k) const
    { return this->m_P_map.at(n)[k]; }

  ////////////////////////
  // Access Eigenvalues //
  ////////////////////////

  /* Get subdivision matrix eigenvalues. */
  const vnl_vector<TReal>& GetSEigenvalues(const unsigned int &n) const
    {  return this->m_S_Eigenvalues_map.at(n); }
  /* Get extended subdivision matrix eigenvalues. */
  const vnl_vector<TReal>& GetAEigenvalues(const unsigned int &n) const
    {  return this->m_A_Eigenvalues_map.at(n); }

  /////////////////////////
  // Access Eigenvectors //
  /////////////////////////

  /* Get subdivision matrix eigenvectors (columnwise). */
  const vnl_matrix<TReal>& GetSEigenvectors(const unsigned int &n) const
    { return this->m_S_Eigenvectors_map.at(n); }
  /* Get extended subdivision matrix eigenvectors (columnwise). */
  const vnl_matrix<TReal>& GetAEigenvectors(const unsigned int &n) const
    { return this->m_A_Eigenvectors_map.at(n); }

  const std::tuple<vnl_vector<unsigned int>,
                   vnl_matrix<TReal>,
                   vnl_matrix<TReal>>& GetSortedEigensystem(const unsigned int &n) const
    { return this->m_Sorted_Eigensystem_map.at(n); }

  /////////////////////////////////////////////////////
  // Uniform Quadratic B-Spline: Barycentric Weights //
  /////////////////////////////////////////////////////

  /*
   * Scaling factor which should be applied to a particular set of Barycentric weights.
   *
   * order: Order of partial derivative being scaled.
   * k: Child patch index.
   * n: Number of required subdivisions.
   */
  static TReal BarycentricBSplineWeightScale(const unsigned int &order,
                                             const unsigned int &k,
                                             const unsigned int &n);

  static vnl_vector_fixed<TReal,12> CalculateBarycentricBSplineWeights(const TParameters &p);
  static vnl_vector_fixed<TReal,12> CalculateBarycentricBSplineWeightsDR(const TParameters &p);
  static vnl_vector_fixed<TReal,12> CalculateBarycentricBSplineWeightsDS(const TParameters &p);
  static vnl_vector_fixed<TReal,12> CalculateBarycentricBSplineWeightsDT(const TParameters &p);

  static vnl_vector_fixed<TReal,12> CalculateBarycentricBezierWeights(const TParameters &p);

  ///////////////////////
  // Thin Plate Energy //
  ///////////////////////

  // BSpline
  const vnl_matrix_fixed<TReal,12,12>& GetBSplineM() const
    { return this->m_BSplineM; }
  const vnl_matrix_fixed<TReal,36,36>& GetBSplineMBlock() const
    { return this->m_BSplineM_block; }
  const vnl_matrix_fixed<TReal,12,12>& GetBSplineMRoot() const
    { return this->m_BSplineM_root; }
  const vnl_matrix_fixed<TReal,36,36>& GetBSplineMRootBlock() const
    { return this->m_BSplineM_root_block; }

  // Bezier
  const vnl_matrix_fixed<TReal,15,15>& GetBezierM() const
    { return this->m_BezierM; }
  const vnl_matrix_fixed<TReal,45,45>& GetBezierMBlock() const
    { return this->m_BezierM_block; }
  const vnl_matrix_fixed<TReal,15,15>& GetBezierMRoot() const
    { return this->m_BezierM_root; }
  const vnl_matrix_fixed<TReal,45,45>& GetBezierMRootBlock() const
    { return this->m_BezierM_root_block; }

  // Conversion
  const vnl_matrix<TReal>& GetBSplineToBezier(const unsigned int &n) const
    { return this->m_BSplineToBezier_map.at(n); }

  ///////////////////////
  // Utility Functions //
  ///////////////////////

  static int         CalculateNumberOfRequiredSubdivisions(const TParameters &p);
  static int         CalculateChildIndex(const TParameters &p);
  static TParameters TransformParametersToPatch(const TParameters &p);
  static bool        VerifyParameters(const TParameters &p);

private:

  ////////////////////////////
  // Backing Datastructures //
  ////////////////////////////

  // Matrices
  std::map<unsigned int,vnl_matrix<TReal>> m_S_map;
  std::map<unsigned int,vnl_matrix<TReal>> m_A_map;
  std::map<unsigned int,vnl_matrix<TReal>> m_B_map;
  std::map<unsigned int,TPickers>          m_P_map;

  // Eigenvalues
  std::map<unsigned int,vnl_vector<TReal>> m_S_Eigenvalues_map;
  std::map<unsigned int,vnl_vector<TReal>> m_A_Eigenvalues_map;

  // Eigenvectors
  std::map<unsigned int,vnl_matrix<TReal>> m_S_Eigenvectors_map;
  std::map<unsigned int,vnl_matrix<TReal>> m_A_Eigenvectors_map;

  std::map<unsigned int,std::tuple<vnl_vector<unsigned int>,
                                   vnl_matrix<TReal>,
                                   vnl_matrix<TReal>>> m_Sorted_Eigensystem_map;
  // Thin plate bspline energy
  const vnl_matrix_fixed<TReal,12,12>      m_BSplineM;
  const vnl_matrix_fixed<TReal,36,36>      m_BSplineM_block;
  const vnl_matrix_fixed<TReal,12,12>      m_BSplineM_root;
  const vnl_matrix_fixed<TReal,36,36>      m_BSplineM_root_block;

  // Thin plate bezier energy
  const vnl_matrix_fixed<TReal,15,15>      m_BezierM;
  const vnl_matrix_fixed<TReal,45,45>      m_BezierM_block;
  const vnl_matrix_fixed<TReal,15,15>      m_BezierM_root;
  const vnl_matrix_fixed<TReal,45,45>      m_BezierM_root_block;

  // Bezier
  std::map<unsigned int,vnl_matrix<TReal>> m_BSplineToBezier_map;

  /////////////////////////
  // Static Calculations //
  /////////////////////////

  // Calculate Matrices
  static vnl_matrix<TReal>           CalculateS(const unsigned int &N);
  static vnl_matrix<TReal>           CalculateS11(const unsigned int &N);
  static vnl_matrix_fixed<TReal,5,5> CalculateS12();
  static vnl_matrix<TReal>           CalculateA(const unsigned int &N);
  static vnl_matrix<TReal>           CalculateS21(const unsigned int &N);
  static vnl_matrix_fixed<TReal,6,5> CalculateS22();
  static vnl_matrix<TReal>           CalculateB(const unsigned int &N);
  static vnl_matrix<TReal>           CalculateP(const unsigned int &N, const unsigned int &k);

  // Calculate Eigenvalues
  static vnl_vector<TReal>         CalculateSEigenvalues(const unsigned int &N);
  static vnl_vector_fixed<TReal,5> CalculateS12Eigenvalues();
  static vnl_vector<TReal>         CalculateAEigenvalues(const unsigned int &N);

  // Calculate Eigenvectors
  static vnl_matrix<TReal>           CalculateSEigenvectors(const unsigned int &N);
  static vnl_matrix<TComplex>        CalculateComplexSEigenvectors(const unsigned int &N);
  static vnl_matrix<TReal>           CalculateS11Eigenvectors(const unsigned int &N);
  static vnl_matrix_fixed<TReal,5,5> CalculateS12Eigenvectors();
  static vnl_matrix<TReal>           CalculateAEigenvectors(const unsigned int &N);

  // Calculate Sorted Eigensystem
  static std::tuple<vnl_vector<unsigned int>,
                    vnl_matrix<TReal>,
                    vnl_matrix<TReal>>
                    CalculateSortedEigensystem(const unsigned int &N);

  // Thin plate energies
  static vnl_matrix_fixed<TReal,12,12> CalculateBSplineThinPlateEnergyMatrix();
  static vnl_matrix_fixed<TReal,36,36> CalculateBSplineThinPlateEnergyMatrixBlock();
  static vnl_matrix_fixed<TReal,12,12> CalculateBSplineThinPlateEnergyMatrixRoot();
  static vnl_matrix_fixed<TReal,36,36> CalculateBSplineThinPlateEnergyMatrixRootBlock();

  static vnl_matrix_fixed<TReal,15,15> CalculateBezierThinPlateEnergyMatrix();
  static vnl_matrix_fixed<TReal,45,45> CalculateBezierThinPlateEnergyMatrixBlock();
  static vnl_matrix_fixed<TReal,15,15> CalculateBezierThinPlateEnergyMatrixRoot();
  static vnl_matrix_fixed<TReal,45,45> CalculateBezierThinPlateEnergyMatrixRootBlock();

  // Utility functions
  static TReal CalculateAlpha(const unsigned int &N);
  static TReal f(const unsigned int &k, const unsigned int &N);
  static TComplex E(const unsigned int &k, const unsigned int &N);

  // Bezier
  static vnl_matrix<TReal> CalculateBSplineToBezierMatrix(const unsigned &N);
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLoopSubdivisionSurfaceMatrices.hxx"
#endif

#endif
