// STD
#include <iostream>
#include <thread>

// Qt
#include <QMessageBox>

// VTK
#include <vtkRenderWindow.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkCamera.h>
#include <vtkTextProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkMatrix4x4.h>
#include <vtkDataSetMapper.h>
#include <vtkTransformFilter.h>
#include <vtkProbeFilter.h>

// VNL
#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>

// Custom
#include <dvGetVTKTransformationMatrixFromITKImage.h>
#include <dvLabeledVTKPointSetReader.h>
#include <dvGetLookupTable.h>

// SiSSR
#include <sissrGetPointsFromITKImage.h>
#include <sissrView.h>

namespace sissr {

/***************
 * CONSTRUCTOR *
 ***************/

View
::View()
{
  this->renderer = vtkSmartPointer< vtkRenderer >::New();
  this->renderer->SetBackground(1.0, 1.0, 1.0);
};

/*********
 * SETUP *
 *********/

void
View
::SetupImageData(const std::string &fileName)
{

  // Error Checking

  if (this->ImageDataHasBeenSetup)
    {
    std::cerr << "WARNING: SetupImageData() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic

  this->itkReader = TImageReader::New();
  this->itkReader->SetFileName( fileName );

  this->itkReader->Update();

  this->itk2vtk = TITK2VTK::New();
  this->itk2vtk->SetInput( this->itkReader->GetOutput() );
  this->itk2vtk->Update();

  // Set Flag

  this->ImageDataHasBeenSetup = true;

}

void
View
::SetupImageInformation()
{
  // Error Checking

  if (this->ImageInformationHasBeenSetup)
    {
    std::cerr << "WARNING: SetupImageInformation() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic

  const auto fMat = vtkSmartPointer<vtkMatrix4x4>::New();
  const auto bMat = vtkSmartPointer<vtkMatrix4x4>::New();

  dv::GetVTKTransformationMatrixFromITKImage<TImage>( this->itkReader->GetOutput(), fMat);

  this->fTrans = vtkSmartPointer<vtkTransform>::New();

  this->fTrans->SetMatrix(  fMat );

  GetPointsFromITKImage<TImage>( this->itkReader->GetOutput(),
                                 this->origin,
                                 this->point1,
                                 this->point2 );
  // Set Flag

  this->ImageInformationHasBeenSetup = true;

}

void
View
::SetupImagePlane(vtkRenderWindowInteractor* interactor)
{

  // Error Checking

  if (this->ImagePlaneHasBeenSetup)
    {
    std::cerr << "WARNING: SetupImagePlanes() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic

  this->planeWidget = vtkSmartPointer<vtkPlaneWidget>::New();

  this->planeWidget->SetInteractor(interactor);
  this->planeWidget->SetOrigin(this->origin.GetVnlVector().data_block());
  this->planeWidget->SetPoint1(this->point1.GetVnlVector().data_block());
  this->planeWidget->SetPoint2(this->point2.GetVnlVector().data_block());
  this->planeWidget->On();

  // Define a model for the "image". Its geometry matches the implicit
  // sphere used to clip the isosurface
  this->planeSource = vtkSmartPointer<vtkPlaneSource>::New();
  this->planeSource->SetOrigin(origin.GetVnlVector().data_block());
  this->planeSource->SetPoint1(point1.GetVnlVector().data_block());
  this->planeSource->SetPoint2(point2.GetVnlVector().data_block());
  this->planeSource->SetResolution(this->PlaneSourceResolution, this->PlaneSourceResolution);

  // This callback will update the plane source, keeping its position in sync
  // with the widget.
  this->callback = vtkSmartPointer<PlaneWidgetCallback>::New();
  this->callback->window = this;
  this->planeWidget->AddObserver( vtkCommand::InteractionEvent, this->callback );

  // vtkProbeFilter will sample the values of the image data at the polydata positions.
  const auto probe = vtkSmartPointer<vtkProbeFilter>::New();
  probe->SetInputConnection( planeSource->GetOutputPort() );
  probe->SetSourceData( this->itk2vtk->GetOutput() );
  probe->Update();

  // Define a suitable grayscale lut
  const auto bwLut = vtkSmartPointer<vtkLookupTable>::New();
  bwLut->SetTableRange(-1024, 2048);
  bwLut->SetSaturationRange(0, 0);
  bwLut->SetHueRange(0, 0);
  bwLut->SetValueRange (.2, 1);
  bwLut->Build();

  // Create a mapper for the sampled image data.
  const auto imageSliceMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  imageSliceMapper->SetInputConnection( probe->GetOutputPort());
  imageSliceMapper->SetScalarRange(-1024, 2048);
  imageSliceMapper->SetLookupTable(bwLut);

  this->imageActor = vtkSmartPointer<vtkActor>::New();
  this->imageActor->SetMapper(imageSliceMapper);

  renderer->AddActor( this->imageActor );

  // Set Flag

  this->ImagePlaneHasBeenSetup = true;

}

void
View
::SetupCandidates(std::string fileName)
{

  // Error Checking

  if (this->CandidatesHaveBeenSetup)
    {
    std::cerr << "WARNING: SetupCandidates() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic
// TODO
//  this->candidateReader = TVTKMeshReader::New();
  this->candidateMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->candidateActor  = vtkSmartPointer<vtkActor>::New();

// TODO
//  this->candidateReader->SetFileName( fileName.c_str() );
//  this->candidateReader->Update();
//  this->candidateMapper->SetInputConnection( this->candidateReader->GetOutputPort() );
  const auto candidates = dv::LabeledVTKPointSetReader( fileName );
  this->candidateMapper->SetInputData( candidates );
  this->candidateActor->SetMapper( this->candidateMapper );
  this->candidateActor->GetProperty()->SetPointSize( 5 );

  // Set Flag

  this->CandidatesHaveBeenSetup = true;

}

void
View
::SetupModel(const std::string fileName)
{

  // Error Checking

  if (this->ModelHasBeenSetup)
    {
    std::cerr << "WARNING: SetupModel() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic

  this->modelReader = TVTKMeshReader::New();

  this->modelEdges    = vtkSmartPointer<vtkExtractEdges>::New();
  this->modelTubes    = vtkSmartPointer<vtkTubeFilter>::New();
  this->modelVertices = vtkSmartPointer<vtkGlyph3D>::New();

  this->modelLoop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();

  this->modelTubes->SetRadius( this->WireframeEdgeRadius );
  this->modelTubes->SetNumberOfSides( this->WireframeNumberOfSides );

  this->modelVertexGlyph = vtkSmartPointer<vtkSphereSource>::New();
  this->modelVertexGlyph->SetRadius( this->WireframeVertexRadius );
  this->modelVertices->SetSourceConnection(this->modelVertexGlyph->GetOutputPort());

  this->modelWireMapper     = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->modelVerticesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->modelLoopMapper     = vtkSmartPointer<vtkPolyDataMapper>::New();

  this->modelWireActor     = vtkSmartPointer<vtkActor>::New();
  this->modelVerticesActor = vtkSmartPointer<vtkActor>::New();
  this->modelLoopActor     = vtkSmartPointer<vtkActor>::New();
  this->modelAssembly      = vtkSmartPointer<vtkAssembly>::New();

  this->modelReader->SetFileName( fileName.c_str() );

  this->modelEdges->SetInputConnection(      this->modelReader->GetOutputPort() );
  this->modelTubes->SetInputConnection(      this->modelEdges->GetOutputPort()  );
  this->modelWireMapper->SetInputConnection( this->modelTubes->GetOutputPort()  );
  this->modelWireActor->SetMapper(           this->modelWireMapper              );

  this->modelWireActor->GetProperty()->SetColor( this->WireframeColor );

  this->modelVertices->SetInputConnection(           this->modelReader->GetOutputPort() );
  this->modelVerticesMapper->SetInputConnection(     this->modelVertices->GetOutputPort() );
  this->modelVerticesActor->SetMapper(               this->modelVerticesMapper );
  this->modelVerticesActor->GetProperty()->SetColor( this->WireframeColor );

  this->modelLoop->SetInputConnection( this->modelReader->GetOutputPort() );
  this->modelLoop->SetNumberOfSubdivisions( this->NumberOfSubdivisions );
  this->modelLoop->Update();
  this->modelLoopMapper->SetInputConnection( this->modelLoop->GetOutputPort() );
  this->modelLoopActor->SetMapper( this->modelLoopMapper );

//  this->modelLoopActor->GetProperty()->LightingOff();

  this->modelAssembly->AddPart( this->modelLoopActor );
  this->modelAssembly->AddPart( this->modelWireActor );
  this->modelAssembly->AddPart( this->modelVerticesActor );

  this->phaseAnnotation = vtkSmartPointer<vtkTextActor>::New();
  this->phaseAnnotation->SetInput( "" );
  this->phaseAnnotation->SetPosition2( 40, 40 );
  this->phaseAnnotation->GetTextProperty()->SetFontSize( 24 );
  this->phaseAnnotation->GetTextProperty()->SetColor( 0.0, 0.0, 0.0 );
  this->renderer->AddActor2D( this->phaseAnnotation );

  this->modelColorbarMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->modelColorbarActor = vtkSmartPointer<vtkScalarBarActor>::New();
  this->modelColorbarActor->SetMapper( this->modelColorbarMapper );
  this->modelColorbarActor->SetNumberOfLabels(this->NumberOfColorbarLabels);
  this->modelColorbarActor->GetTitleTextProperty()->SetColor(0.0, 0.0, 0.0);
  this->modelColorbarActor->GetLabelTextProperty()->SetColor(0.0, 0.0, 0.0);
  this->modelColorbarActor->GetTitleTextProperty()->ShadowOff();
  this->modelColorbarActor->GetLabelTextProperty()->ItalicOff();
  this->modelColorbarActor->GetTitleTextProperty()->BoldOff();
  this->modelColorbarActor->SetLabelFormat("%.2f");

  // Set Flag

  this->ModelHasBeenSetup = true;

}

void
View
::SetupResiduals(const std::string fileName)
{

  // Error Checking

  if (this->ResidualsHaveBeenSetup)
    {
    std::cerr << "WARNING: SetupResiduals() has already been called.\n"
              << "Returning." << std::endl;
    return;
    }

  // Logic

  this->modelResidualsReader = TVTKResidualsReader::New();
  this->modelResidualsTubes  = vtkSmartPointer<vtkTubeFilter>::New();
  this->modelResidualsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->modelResidualsActor  = vtkSmartPointer<vtkActor>::New();

  this->modelResidualsReader->SetFileName( fileName.c_str() );
  this->modelResidualsTubes->SetRadius( this->ResidualsEdgeRadius );
  this->modelResidualsTubes->SetNumberOfSides( this->ResidualsNumberOfSides );
  this->modelResidualsTubes->CappingOn();
  this->modelResidualsTubes->SetInputConnection( this->modelResidualsReader->GetOutputPort() );
  this->modelResidualsMapper->SetInputConnection( this->modelResidualsTubes->GetOutputPort() );
  this->modelResidualsActor->SetMapper( this->modelResidualsMapper );
  this->modelResidualsActor->GetProperty()->SetColor( this->ResidualsColor );
  this->modelResidualsReader->Update();

  this->modelResidualsVertices    = vtkSmartPointer<vtkGlyph3D>::New();
  this->modelResidualsVertexGlyph = vtkSmartPointer<vtkSphereSource>::New();

  this->modelResidualsVertexGlyph->SetRadius(
    this->ResidualsVertexRadius );

  this->modelResidualsVertices->SetSourceConnection(
    this->modelResidualsVertexGlyph->GetOutputPort() );

  this->modelResidualsVerticesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->modelResidualsVerticesActor  = vtkSmartPointer<vtkActor>::New();

  this->modelResidualsVertices->SetInputConnection(
    this->modelResidualsReader->GetOutputPort() );
  this->modelResidualsVerticesMapper->SetInputConnection(
    this->modelResidualsVertices->GetOutputPort() );
  this->modelResidualsVerticesActor->SetMapper( this->modelResidualsVerticesMapper );
  this->modelResidualsVerticesActor->GetProperty()->SetColor( this->ResidualsColor );

  this->modelAssembly->AddPart( this->modelResidualsActor );
  this->modelAssembly->AddPart( this->modelResidualsVerticesActor );

  // Set Flag

  this->ResidualsHaveBeenSetup = true;

}

/**************
 * VISIBILITY *
 **************/

void
View
::SetImagePlanesVisible(const bool &visible)
{

  if (nullptr == this->planeWidget)
    {
    std::cerr << "Warning: planeWidget is null...returning." << std::endl;
    return;
    }

  this->planeWidget->SetEnabled(visible);
  visible ? this->renderer->AddActor( this->imageActor ) : this->renderer->RemoveActor( this->imageActor );

}

void
View
::SetCandidatesVisible(const bool &visible)
{

  if (nullptr == this->candidateActor)
    {
    std::cerr << "WARNING: Candidate actor is null.\n"
              << "Returning." << std::endl;
    return;
    }

  if (visible)
    {
    this->renderer->AddActor( this->candidateActor );
    }
  else
    {
    this->renderer->RemoveActor( this->candidateActor );
    }

}

void
View
::SetModelVisible(const bool &visible)
{

  if (nullptr == this->modelAssembly)
    {
    std::cerr << "WARNING: Model assembly is null.\n"
              << "Returning." << std::endl;
    return;
    }

  if (visible)
    {
    this->renderer->AddActor( this->modelAssembly );
    }
  else
    {
    this->renderer->RemoveActor( this->modelAssembly );
    }

}

void
View
::SetModelWiresVisible(const bool &visible)
{

  if (nullptr == this->modelAssembly ||
      nullptr == this->modelWireActor ||
      nullptr == this->modelVerticesActor)
    {
    return;
    }

  if (visible)
    {
    this->modelAssembly->AddPart( this->modelWireActor );
    this->modelAssembly->AddPart( this->modelVerticesActor );
    }
  else
    {
    this->modelAssembly->RemovePart( this->modelWireActor );
    this->modelAssembly->RemovePart( this->modelVerticesActor );
    }
}

void
View
::SetModelSurfaceVisible(const bool &visible)
{

  if (nullptr == this->modelAssembly ||
      nullptr == this->modelLoopActor)
    {
    return;
    }

  if (visible)
    {
    this->modelAssembly->AddPart( this->modelLoopActor );
    }
  else
    {
    this->modelAssembly->RemovePart( this->modelLoopActor );
    }

}

void
View
::SetColorbarVisible(const bool &visible)
{

  if (nullptr == this->modelColorbarActor)
    {
    return;
    }

  if (visible)
    {
    this->modelColorbarActor->SetLookupTable( this->modelLUT );
    this->renderer->AddActor2D( this->modelColorbarActor );
    }
  else
    {
    this->renderer->RemoveActor2D( this->modelColorbarActor );
    }

}

void
View
::SetResidualsVisible(const bool &visible)
{

  if (nullptr == this->modelAssembly ||
      nullptr == this->modelResidualsActor)
    {
    return;
    }

  if (visible)
    {
    this->modelAssembly->AddPart( this->modelResidualsActor );
    this->modelAssembly->AddPart( this->modelResidualsVerticesActor );
    }
  else
    {
    this->modelAssembly->RemovePart( this->modelResidualsActor );
    this->modelAssembly->RemovePart( this->modelResidualsVerticesActor );
    }
}

// FIXME
void
View
::UpdateLookupTable()
{
  if (!this->ModelHasBeenSetup)
    {
    return;
    }

  if (nullptr == this->modelLUT)
    {
    this->modelLUT = vtkSmartPointer<vtkLookupTable>::New();
    }

  ///////////////////////////////////////////////
  // Build Lookup Table From Transfer Function //
  ///////////////////////////////////////////////

  const auto ctf = this->CalculateMeshTransferFunction();

  const size_t N = 256; 
  this->modelLUT->SetNumberOfTableValues(N);
  this->modelLUT->SetTableRange(this->CB_State.Min,
                                this->CB_State.Max);
//  this->modelLUT->UseAboveRangeColorOn();
//  this->modelLUT->UseBelowRangeColorOn();
  this->modelLUT->Build();
 
  for(size_t i = 0; i < N; ++i)
    {
    double *rgb;
    rgb = ctf->GetColor(static_cast<double>(i)/N);
    this->modelLUT->SetTableValue(i,rgb[0], rgb[1], rgb[2]);
    }

  const double gray = 0.3;

  this->modelLUT->SetBelowRangeColor( gray, gray, gray, 1.0 ); // Gray
  this->modelLUT->SetAboveRangeColor( gray, gray, gray, 1.0 ); // Gray

  /////////////////
  // Loop Mapper //
  /////////////////

  const auto lut = dv::LUT::Rainbow();
  this->modelLoopMapper->SetLookupTable( lut );
  this->modelLoopMapper->SetScalarRange(0, 8);

//  this->modelLoopMapper->SetScalarRange(this->CB_State.Min,
//                                        this->CB_State.Max);
//  this->modelLoopMapper->SetLookupTable( this->modelLUT );

  ///////////
  // Actor //
  ///////////

  this->modelColorbarActor->SetTitle(this->CB_State.Title.c_str());
  this->modelColorbarActor->SetLookupTable( this->modelLUT );

}

/*****************
 * UPDATE SOURCE *
 *****************/

void
View
::UpdatePlanesSource(const std::string &fileName)
{

  itkAssertOrThrowMacro( (nullptr != this->itk2vtk),
                         "ERROR: itk2vtk is null.");

  this->itkReader->SetFileName( fileName.c_str() );
  this->itk2vtk->Update();

}

void
View
::UpdateCandidatesSource(const std::string &fileName)
{

//TODO
//  itkAssertOrThrowMacro( (nullptr != this->candidateReader),
//                         "ERROR: Candidate reader is null.");
//
//  this->candidateReader->SetFileName( fileName.c_str() );
//  this->candidateReader->Update();
  const auto candidates = dv::LabeledVTKPointSetReader( fileName );
  this->candidateMapper->SetInputData( candidates );

}

void
View
::UpdateModelSource(const std::string &fileName)
{

  itkAssertOrThrowMacro( (nullptr != this->modelReader),
                         "ERROR: Model reader is null.");

  this->modelReader->SetFileName( fileName.c_str() );
  this->modelReader->Update();

  if (nullptr != this->modelCellData)
    {
    this->modelLoop->GetOutput()->GetCellData()->SetScalars( this->modelCellData );
    this->UpdateLookupTable();
    }

}

void
View
::UpdateResidualsSource(const std::string &fileName)
{

  if (nullptr == this->modelResidualsReader)
    {
    std::cerr << "WARNING: Model residuals reader is null." << std::endl;
    return;
    }

  this->modelResidualsReader->SetFileName( fileName.c_str() );
  this->modelResidualsReader->Update();

}

/*************
 * Utilities *
 *************/

vtkSmartPointer<vtkColorTransferFunction>
View
::CalculateMeshTransferFunction()
{
  const auto ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
  ctf->SetScaleToLinear();
  ctf->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
  ctf->AddRGBPoint(0.5, 1.0, 0.0, 0.0);
  ctf->AddRGBPoint(1.0, 1.0, 1.0, 0.0);
  return ctf;
}

vtkSmartPointer<vtkColorTransferFunction>
View
::CalculateImageTransferFunction(const double &wMin,
                                 const double &wMax,
                                 const double &mMin,
                                 const double &mMax)
{

  auto transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  const double &WindowRange = wMax - wMin;
  const double &LowerBoundOuter = (mMin-1-wMin)/WindowRange;
  const double &LowerBoundInner = (mMin-wMin)/WindowRange;
  const double &UpperBoundInner = (mMax-wMin)/WindowRange;
  const double &UpperBoundOuter = (mMax+1-wMin)/WindowRange;
  transferFunction->AddRGBPoint(wMin,
                                0.0,
                                0.0,
                                0.0);
  transferFunction->AddRGBPoint(mMin-1,
                                LowerBoundOuter,
                                LowerBoundOuter,
                                LowerBoundOuter);
  transferFunction->AddRGBPoint(mMin,
                                1.0,
                                LowerBoundInner,
                                LowerBoundInner);
  transferFunction->AddRGBPoint(mMax,
                                1.0,
                                UpperBoundInner,
                                UpperBoundInner);
  transferFunction->AddRGBPoint(mMax+1,
                                UpperBoundOuter,
                                UpperBoundOuter,
                                UpperBoundOuter);
  transferFunction->AddRGBPoint(wMax,
                                1.0,
                                1.0,
                                1.0);
  return transferFunction;

}

void
View
::PlaneWidgetDidMove()
{
  this->planeSource->SetOrigin(this->planeWidget->GetOrigin());
  this->planeSource->SetPoint1(this->planeWidget->GetPoint1());
  this->planeSource->SetPoint2(this->planeWidget->GetPoint2());
}

} // namespace sissr
