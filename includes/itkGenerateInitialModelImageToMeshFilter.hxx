#ifndef itkGenerateInitialModelImageToMeshFilter_hxx
#define itkGenerateInitialModelImageToMeshFilter_hxx

#include "itkGenerateInitialModelImageToMeshFilter.h"

// ITK
#include <itkQuadEdgeMesh.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkCuberilleImageToMeshFilter.h>
#include <itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h>

// CGAL
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>

// Custom
#include <itkCleanSegmentationImageFilter.h>
#include <itkRefineValenceThreeVerticesQuadEdgeMeshFilter.h>

// SiSSR
#include <sissrEdge_preserving_LindstromTurk_placement.h>
#include <sissrEdge_preserving_midpoint_placement.h>
#include <sissrITKMeshToCGALSurfaceMesh.h>
#include <sissrCGALSurfaceMeshToITKMesh.h>

namespace itk
{

template <typename TInputImage, typename TOutputMesh>
GenerateInitialModelImageToMeshFilter<TInputImage, TOutputMesh>
::GenerateInitialModelImageToMeshFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->m_LVClosingRadius = 3;
  this->m_GeneralClosingRadius = 3;
  this->m_DecimationTechnique = CGALDecimationTechnique::LindstromTurk;
  this->m_PreserveEdges = true;
}

template <typename TInputImage, typename TOutputMesh>
void
GenerateInitialModelImageToMeshFilter<TInputImage, TOutputMesh>
::SetInput(const InputImageType * image)
{
  this->ProcessObject::SetNthInput(0, const_cast<InputImageType *>(image));
}

template <typename TInputImage, typename TOutputMesh>
void
GenerateInitialModelImageToMeshFilter<TInputImage, TOutputMesh>
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}


template <typename TInputImage, typename TOutputMesh>
void
GenerateInitialModelImageToMeshFilter<TInputImage, TOutputMesh>
::GenerateData()
{

  const auto image = Superclass::GetInput(0);
  typename OutputMeshType::Pointer mesh = Superclass::GetOutput();

  using TKernel = itk::BinaryBallStructuringElement<InputPixelType, InputImageDimension>;
  using TClose = itk::BinaryMorphologicalClosingImageFilter<InputImageType, InputImageType, TKernel>;
  using TClean = itk::CleanSegmentationImageFilter<InputImageType>;
  using TCuberille = itk::CuberilleImageToMeshFilter< InputImageType, OutputMeshType >;
  using TRefine = itk::RefineValenceThreeVerticesQuadEdgeMeshFilter<OutputMeshType>;
  using TLoop = itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter<OutputMeshType>;

  using TCGALKernel = CGAL::Simple_cartesian<typename TOutputMesh::PixelType>;
  using TCGALPoint = typename TCGALKernel::Point_3;
  using TCGALMesh = CGAL::Surface_mesh<TCGALPoint>;
  namespace SMS = CGAL::Surface_mesh_simplification;
  namespace PMP = CGAL::Polygon_mesh_processing;
  using TCGALEPMPPlacement = SMS::Bounded_normal_change_placement<SMS::Edge_preserving_midpoint_placement<TCGALMesh,typename OutputMeshType::PixelType>>;
  using TCGALEPLTPlacement = SMS::Bounded_normal_change_placement<SMS::Edge_preserving_LindstromTurk_placement<TCGALMesh,typename OutputMeshType::PixelType>>;
  using TCGALMPPlacement = SMS::Bounded_normal_change_placement<SMS::Midpoint_placement<TCGALMesh>>;
  using TCGALLTPlacement = SMS::Bounded_normal_change_placement<SMS::LindstromTurk_placement<TCGALMesh>>;
  using TCGALLTCost = SMS::LindstromTurk_cost<TCGALMesh>;

  TKernel closeKernel;
  closeKernel.SetRadius(this->GetGeneralClosingRadius());
  closeKernel.CreateStructuringElement();

  const auto clean = TClean::New();
  clean->SetInput( image );

  std::array<typename TClose::Pointer, 8> closing;
  for (size_t i = 0; i < 8; ++i) {
    closing[i] = TClose::New();
    closing[i]->SetKernel(closeKernel);
    closing[i]->SetForegroundValue( i + 2 );
    if (i > 0) {
      closing[i]->SetInput( closing[i - 1]->GetOutput() );
    } else {
      closing[i]->SetInput( clean->GetOutput() );
    }
  }

  TKernel lvCloseKernel;
  lvCloseKernel.SetRadius(this->GetLVClosingRadius());
  lvCloseKernel.CreateStructuringElement();

  const auto closing_lv = TClose::New();
  closing_lv->SetInput( closing.back()->GetOutput() );
  closing_lv->SetKernel( lvCloseKernel );
  closing_lv->SetForegroundValue( 1 );
  // DO NOT REMOVE CALL TO Update()
  // Removing causes failures for a small subset of inputs
  closing_lv->Update();

  const auto cuberille = TCuberille::New();
  cuberille->SetInput(closing_lv->GetOutput());
  cuberille->GenerateTriangleFacesOn();
  // cuberille->RemoveProblematicPixelsOn(); // Method not available in current ITK version
  cuberille->ProjectVerticesToIsoSurfaceOff();
  cuberille->SavePixelAsCellDataOn();
  cuberille->Update();

  // CONVERT ITK TO CGAL
  auto surface_mesh = sissr::ITKMeshToCGALSurfaceMesh<TOutputMesh, TCGALMesh>( cuberille->GetOutput() );

  // VERIFY AND DECIMATE
  itkAssertOrThrowMacro(CGAL::is_triangle_mesh(surface_mesh), "Input geometry is not triangulated.");

  SMS::Edge_count_stop_predicate<TCGALMesh> stop(this->GetNumberOfCellsInDecimatedMesh());

  if (m_PreserveEdges) {

    switch (this->m_DecimationTechnique) {

      case CGALDecimationTechnique::Midpoint:

        SMS::edge_collapse(
          surface_mesh
          , stop
          , CGAL::parameters::get_placement(TCGALEPMPPlacement())
        );
        break;

      case CGALDecimationTechnique::LindstromTurk:

        SMS::edge_collapse(
            surface_mesh,
            stop,
            CGAL::parameters::get_cost(TCGALLTCost())
            .get_placement(TCGALEPLTPlacement())
            );
        break;

      default:

        itkAssertOrThrowMacro(false, "Unrecognized decimation technique.");
        break;

    }

  } else {

    switch (this->m_DecimationTechnique) {

      case CGALDecimationTechnique::Midpoint:

        SMS::edge_collapse(
          surface_mesh
          , stop
          , CGAL::parameters::get_placement(TCGALMPPlacement())
        );
        break;

      case CGALDecimationTechnique::LindstromTurk:

        SMS::edge_collapse(
            surface_mesh,
            stop,
            CGAL::parameters::get_cost(TCGALLTCost())
            .get_placement(TCGALLTPlacement())
            );
        break;

      default:

        itkAssertOrThrowMacro(false, "Unrecognized decimation technique.");
        break;

    }

  }

  PMP::keep_largest_connected_components(surface_mesh, 1);

  surface_mesh.collect_garbage();

  // CONVERT CGAL TO ITK
  const auto o_mesh = sissr::CGALSurfaceMeshToITKMesh<TCGALMesh, TOutputMesh>(surface_mesh);

  const auto refine = TRefine::New();
  refine->SetInput( o_mesh );

  const auto loop = TLoop::New();
  loop->SetInput( refine->GetOutput() );
  loop->Update();

  mesh->Graft( loop->GetOutput() );

}

} // end namespace itk

#endif // itkGenerateInitialModelImageToMeshFilter_hxx
