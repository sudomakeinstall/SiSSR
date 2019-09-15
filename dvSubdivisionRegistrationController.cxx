#ifndef dvSubdivisionRegistrationController_cxx
#define dvSubdivisionRegistrationController_cxx

#include <dvSubdivisionRegistrationController.h>

// Qt
#include <QMessageBox>

// VTK
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkCamera.h>
#include <vtkTriangle.h>
#include <vtkLine.h>
#include <vtkPlane.h>

// ITK
#include <itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkMeshFileReader.h>
#include <itkTimeProbe.h>
#include <itkIterativeTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkQuadEdgeMesh.h>
#include <itkQuadEdgeMeshDecimationCriteria.h>
#include <itkQuadricDecimationQuadEdgeMeshFilter.h>
#include <itkSquaredEdgeLengthDecimationQuadEdgeMeshFilter.h>
#include <itkSmoothingQuadEdgeMeshFilter.h>
#include <itkQuadEdgeMeshParamMatrixCoefficients.h>
#include <itkMeshFileReader.h>
#include <itkMeshFileWriter.h>
#include <itkPointsLocator.h>

// DVCppUtils
#include <dvProgress.h>
#include <dvSignedDistanceToPlane.h>
#include <dvCyclicMean.h>
#include <dvConvertLabelsToMesh.h>
#include <dvGenerateInitialModel.h>

// Custom
#include <itkAdditiveGaussianNoiseQuadEdgeMeshFilter.h>
#include <dvRegisterMeshToPointSet.h>
#include <dvRefineValenceThreeVertices.h>
#include <dvCalculateResidualMesh.h>
#include <dvCalculateSurfaceAreas.h>
#include <dvCalculateTriangleCenters.h>

// Standard
#include <iostream>
#include <thread>

namespace dv
{

/*
  CONSTRUCTOR
*/

SubdivisionRegistrationController
::SubdivisionRegistrationController(int argc, char **argv) :
  FileTree(argv[1], argv[2])
{

  // Determine current state
  this->DetermineState();
  this->Deserialize();

  // Basic setup for Qt Widgets
  this->ui = new Ui_QtVTKRenderWindows;
  this->ui->setupUi(this);
  const auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  this->ui->imageWindow->setRenderWindow(renderWindow);

  this->ui->imageWindow->renderWindow()->AddRenderer(this->window.renderer);
  this->ui->imageWindow->renderWindow()->SetAlphaBitPlanes(1);

  this->SetupSliderRanges();
  this->SetupValueLabels();
  this->SetupSlots();

  this->ui->toolBox->setCurrentIndex(static_cast<int>(this->State.GetCurrentState()));

  // Setup radio buttons
  this->ui->cellDataButtonNone->setChecked(false);
  this->ui->cellDataButtonSQUEEZ->setChecked(false);
  this->ui->cellDataButtonResidual->setChecked(false);
  this->ui->cellDataButtonSegmentation->setChecked(false);
  switch (this->State.CellDataToDisplay)
    {
    case CellData::NONE:
      this->ui->cellDataButtonNone->setChecked(true);
      break;
    case CellData::SQUEEZ:
      this->ui->cellDataButtonSQUEEZ->setChecked(true);
      break;
    case CellData::RESIDUALS:
      this->ui->cellDataButtonResidual->setChecked(true);
      break;
    case CellData::SEGMENTATION:
      this->ui->cellDataButtonSegmentation->setChecked(true);
      break;
    }

  // Setup IMAGE pipeline
  this->SetupImage();
  this->SetupImageVolume();
  this->SetupImagePlanes();
  this->SetupReslicePlanes();
  this->SetupCandidates();
  this->SetupModel();

  this->ui->frameSlider->setValue(this->State.CurrentFrame);

  this->Render();

  std::set<std::string> arguments;
  for (int i = 2; i < argc; ++i)
    {
    arguments.emplace( std::string(argv[i]) );
    }

  if (arguments.find(std::string("--candidates")) != arguments.end())
    {
    this->CalculateBoundaryCandidates();
    }

  if (arguments.find(std::string("--register")) != arguments.end())
    {
    this->Register();
    }

  if (arguments.find(std::string("--segment")) != arguments.end())
    {
    this->CalculateSegmentIDs();
    }

  if (arguments.find(std::string("--areas")) != arguments.end())
    {
    this->CalculateSurfaceAreas();
    }

};

void
SubdivisionRegistrationController
::DetermineState()
{

  ////////////////
  // Plane Data //
  ////////////////

  std::cout << "Plane data..." << std::flush;
  if (this->FileTree.GetNumberOfFiles() > 0)
    {
    this->State.PlaneDataExists = true;
    std::cout << "found." << std::endl;
    }
  else
    {
    this->State.PlaneDataExists = false;
    std::cout << "not found." << std::endl;
    return;
    }

  ////////////////////
  // Candidate Data //
  ////////////////////

  std::cout << "Candidate data..." << std::flush;
  this->State.CandidateDataExists = this->FileTree.CandidateDataExists();
  if (this->State.CandidateDataExists)
    {
    std::cout << "found." << std::endl;
    }
  else
    {
    std::cout << "not found." << std::endl;
    return;
    }

  ////////////////////////
  // Initial Model Data //
  ////////////////////////

  std::cout << "Initial model data..." << std::flush;
  this->State.InitialModelDataExists = itksys::SystemTools::FileExists(this->FileTree.InitialModel,true);
  if (this->State.InitialModelDataExists)
    {
    std::cout << "found." << std::endl;
    }
  else
    {
    std::cout << "not found." << std::endl;
    return;
    }

  ///////////////////////////
  // Registered Model Data //
  ///////////////////////////

  std::cout << "Determining number of registration passes..." << std::flush;

  this->State.NumberOfRegistrationPasses = this->FileTree.NumberOfRegistrationPasses();
  std::cout << this->State.NumberOfRegistrationPasses << "." << std::endl;

  ////////////////////////////
  // Calculated Information //
  ////////////////////////////

  for (unsigned int p = 0; p <= this->FileTree.NumberOfRegistrationPasses(); ++p)
    {
    if (!this->FileTree.ResidualMeshDataExistsForPass(p))
      {
      this->CalculateResidualsForPass(p);
      }
    }

}

void
SubdivisionRegistrationController
::SetupSliderRanges()
{

  this->ui->frameSlider->setRange(      0,this->FileTree.GetNumberOfFiles()-1);

  this->ui->planesDistanceSlider->setRange(this->State.ReslicePlanesDistanceMin * 100,
                                           this->State.ReslicePlanesDistanceMax * 100);
  this->ui->planesDistanceSlider->setValue(this->State.ReslicePlanesDistance * 100);

}

void
SubdivisionRegistrationController
::SetupValueLabels()
{

  this->ui->frameValue->setText(
    QString::fromStdString(std::to_string(this->GetCurrentFrame())) );

  if (this->State.EDFrameHasBeenSet)
    this->ui->edValue->setText(
      QString::fromStdString(std::to_string(this->State.EDFrame))
                              );

  this->ui->planesDistanceValue->setText(
    QString::fromStdString(std::to_string(this->ui->planesDistanceSlider->value())));

}

void
SubdivisionRegistrationController
::SetupSlots()
{

  // Set up action signals and slots

  connect(this->ui->frameSlider,
          SIGNAL(valueChanged(int)), this, SLOT(FrameValueChanged(int)));

  connect(this->ui->jumpToEDButton,
          SIGNAL(pressed()), this, SLOT(JumpToEDButtonPressed()));

  connect(this->ui->edButton,
          SIGNAL(pressed()), this, SLOT(EDButtonPressed()));

  connect(this->ui->calculateCandidatesButton,
          SIGNAL(pressed()), this, SLOT(CalculateBoundaryCandidates()));

  connect(this->ui->toggleImageVolumeButton,
          SIGNAL(pressed()), this, SLOT(ToggleImageVolume()));

  connect(this->ui->togglePlanesButton,
          SIGNAL(pressed()), this, SLOT(ToggleImagePlanes()));

  connect(this->ui->toggleCandidatesButton,
          SIGNAL(pressed()), this, SLOT(ToggleCandidates()));

  connect(this->ui->toggleModelButton,
          SIGNAL(pressed()), this, SLOT(ToggleModel()));

  connect(this->ui->toggleReslicePlanes,
          SIGNAL(pressed()), this, SLOT(ToggleReslicePlanes()));

  connect(this->ui->toggleWiresButton,
          SIGNAL(pressed()), this, SLOT(ToggleModelWires()));

  connect(this->ui->toggleSurfaceButton,
          SIGNAL(pressed()), this, SLOT(ToggleModelSurface()));

  connect(this->ui->toggleColorbarButton,
          SIGNAL(pressed()), this, SLOT(ToggleColorbar()));

  connect(this->ui->toggleResidualsButton,
          SIGNAL(pressed()), this, SLOT(ToggleResiduals()));

  connect(this->ui->toolBox,
          SIGNAL(currentChanged(int)), this, SLOT(CurrentPageChanged(int)));

  connect(this->ui->frameIncrement,
          SIGNAL(pressed()), this, SLOT(IncrementFrame()));

  connect(this->ui->frameDecrement,
          SIGNAL(pressed()), this, SLOT(DecrementFrame()));

  connect(this->ui->registerButton,
          SIGNAL(pressed()), this, SLOT(Register()));

  connect(this->ui->writeScreenshots,
          SIGNAL(pressed()), this, SLOT(WriteScreenshots()));

  connect(this->ui->cellDataButtonNone,
          SIGNAL(clicked()), this, SLOT(UpdateCellData()));

  connect(this->ui->cellDataButtonSQUEEZ,
          SIGNAL(clicked()), this, SLOT(UpdateCellData()));

  connect(this->ui->cellDataButtonResidual,
          SIGNAL(clicked()), this, SLOT(UpdateCellData()));

  connect(this->ui->planesDistanceSlider,
          SIGNAL(valueChanged(int)), this, SLOT(PlanesDistanceValueChanged(int)));

  connect(this->ui->calculateSegmentIDsButton,
          SIGNAL(pressed()), this, SLOT(CalculateSegmentIDs()));

}

void
SubdivisionRegistrationController
::PlanesDistanceValueChanged(int value)
{
  this->State.ReslicePlanesDistance = static_cast<double>(value) / 100.0;
  this->ui->planesDistanceSlider->setValue( value );
  this->ui->planesDistanceValue->setText(
    QString::fromStdString(std::to_string(static_cast<float>(value) / 100.0f))
                                        );
  this->UpdateReslicePlanesPlacement();
  this->Render();
}

void
SubdivisionRegistrationController
::EDButtonPressed()
{

  this->State.EDFrame = this->GetCurrentFrame();
  this->ui->edValue->setText(
    QString::fromStdString(std::to_string(this->State.EDFrame)));

  this->State.EDFrameHasBeenSet = true;
  this->UpdateCurrentIndex();

}

void
SubdivisionRegistrationController
::JumpToEDButtonPressed()
{
  if (this->State.EDFrameHasBeenSet)
    {
    this->ui->frameSlider->setValue(this->State.EDFrame);
    }
}

void
SubdivisionRegistrationController
::UpdateCurrentIndex()
{

  const auto CURRENT_STATE = this->State.GetCurrentState();
  const int index = static_cast<int>(CURRENT_STATE);

  if (index != this->ui->toolBox->currentIndex())
    {
    this->ui->toolBox->setCurrentIndex(index);
    }

  if (dv::State::ORIENTATION_CAPTURED == CURRENT_STATE)
    {
    this->SetupModel();
    }

}

void
SubdivisionRegistrationController
::IncrementFrame()
{

  int val = this->GetCurrentFrame() + 1;
  val %= this->FileTree.GetNumberOfFiles();
  this->ui->frameSlider->setValue( val );

}

void
SubdivisionRegistrationController
::DecrementFrame()
{

  int val = this->GetCurrentFrame() - 1;
  if (val < 0) val += this->FileTree.GetNumberOfFiles();
  this->ui->frameSlider->setValue( val );

}

void
SubdivisionRegistrationController
::CurrentPageChanged(int index)
{

  dv::State requestedState = static_cast<dv::State>(index);

  if (requestedState <= this->State.GetCurrentState())
    return;

  QMessageBox alert;
  alert.setText("This tab's analysis must be completed before proceeding.");
  alert.exec();

  this->ui->toolBox->setCurrentIndex( static_cast<int>(this->State.GetCurrentState()));

}

void
SubdivisionRegistrationController
::FrameValueChanged(int value)
{

  itkAssertOrThrowMacro(0 <= value, "Frame may not be negative.");
  this->State.CurrentFrame = static_cast<unsigned int>(value);
  this->ui->frameValue->setText(QString::fromStdString(std::to_string(value)));

  /**********
   * Planes *
   **********/

  if (this->State.ImagePlanesAreVisible)
    {
    this->window.UpdatePlanesSource(this->FileTree.ImagePathForFrame(value));
    }

  /**************
   * Candidates *
   **************/

  if (this->State.CandidatesAreVisible)
    {
    this->window.UpdateCandidatesSource(this->FileTree.CandidatePathForFrame(value));
    }

  /*********
   * Model *
   *********/

  this->UpdateCellData();
  this->UpdateModelTransform();
  this->Render();

  this->UpdateModelTransform();

  /***************
   * Annotations *
   ***************/

  this->UpdateAnnotations();

  /**********
   * Render *
   **********/

  this->Render();

}

void
SubdivisionRegistrationController
::UpdateAnnotations()
{

  if (this->State.GetCurrentState() == dv::State::MODEL_POSITIONED)
    {
      if (this->GetCurrentFrame() == this->State.EDFrame)
        {
        this->window.SetPhaseAnnotation("End Diastole");
        }
      else
        {
        this->window.SetPhaseAnnotation("");
        }
    }
}

void
SubdivisionRegistrationController
::UpdateModelTransform()
{

  if ( !(this->State.InitialModelDataExists && this->State.ModelHasBeenSetup) )
    {
    return;
    }

  if ( !this->State.ModelIsVisible )
    {
    this->window.SetModelVisible(false);
    this->window.SetColorbarVisible(false);
    return;
    }

  // Residual File
  if (this->State.ResidualsAreVisible)
    {
    const auto file
      = this->FileTree.ResidualMeshPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses,
          this->GetCurrentFrame()
                                                  );
    this->window.UpdateResidualsSource( file );
    }

  if (0 == this->State.NumberOfRegistrationPasses)
    {
    this->window.SetModelVisible(true);
    this->window.SetColorbarVisible(false);
    return;
    }

  // Model Surface File
  if (this->State.ModelSurfaceIsVisible || this->State.ModelWiresAreVisible)
    {
    const auto file
      = this->FileTree.RegisteredModelPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses - 1,
          this->GetCurrentFrame()
                                                         );
    this->window.UpdateModelSource( file );
    }

  this->window.SetModelVisible(true);
  this->window.SetColorbarVisible(CellData::NONE != this->State.CellDataToDisplay &&
                                  this->State.ColorbarIsVisible);

}

void
SubdivisionRegistrationController
::UpdateReslicePlanesPlacement()
{
  this->window.UpdateReslicePlanesDistance(this->State.ReslicePlanesDistance);
}

void
SubdivisionRegistrationController
::Render()
{
  this->ui->imageWindow->renderWindow()->Render();
}

void
SubdivisionRegistrationController
::CalculateBoundaryCandidates()
{

  std::cout << "Calculating boundary candidates..." << std::endl;
  auto progress = dv::Progress( this->FileTree.GetNumberOfFiles() );

  itk::TimeProbe clock;
  clock.Start();

  for (unsigned int file = 0; file < this->FileTree.GetNumberOfFiles(); ++file)
    {

    const auto input = this->FileTree.SegmentationPathForFrame(file);
    const auto output = this->FileTree.CandidatePathForFrame(file);

    std::set<unsigned short> labels;
    labels.emplace( 1 );

    ConvertLabelsToMesh<3, unsigned short, double>(
      input,
      labels,
      output
    );

    progress.UnitCompleted();

    }

  this->State.CandidateDataExists = true;
  this->UpdateCurrentIndex();

  clock.Stop();
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;

  this->SetupCandidates();

}

void
SubdivisionRegistrationController
::ToggleImageVolume()
{
  
  this->State.ImageVolumeIsVisible = !this->State.ImageVolumeIsVisible;

  if (this->State.ImageVolumeIsVisible)
    {
    const auto file = this->FileTree.ImagePathForFrame( this->GetCurrentFrame() );
    this->window.UpdatePlanesSource( file );
    }

  this->window.SetImageVolumeVisible( this->State.ImageVolumeIsVisible );

  this->Render();

}

void
SubdivisionRegistrationController
::ToggleImagePlanes()
{

  this->State.ImagePlanesAreVisible = !this->State.ImagePlanesAreVisible;

  if (this->State.ImagePlanesAreVisible)
    {
    const auto file = this->FileTree.ImagePathForFrame( this->GetCurrentFrame() );
    this->window.UpdatePlanesSource( file );
    }

  this->window.SetImagePlanesVisible(this->State.ImagePlanesAreVisible);
  this->Render();

}

void
SubdivisionRegistrationController
::ToggleReslicePlanes()
{

  this->State.ReslicePlanesAreVisible = !this->State.ReslicePlanesAreVisible;
  this->window.SetReslicePlanesVisible(this->State.ReslicePlanesAreVisible);
  this->Render();

}
  
void
SubdivisionRegistrationController
::ToggleCandidates()
{

  if (!this->State.CandidateDataExists)
    {
    QMessageBox alert;
    alert.setText("Candidates have not been calculated.");
    alert.exec();
    return;
    }

  this->State.CandidatesAreVisible = !this->State.CandidatesAreVisible;

  if (this->State.CandidatesAreVisible)
    {
    const auto file = this->FileTree.CandidatePathForFrame( this->GetCurrentFrame() );
    this->window.UpdateCandidatesSource( file );
    }

  this->window.SetCandidatesVisible(this->State.CandidatesAreVisible);
  this->Render();

}

void
SubdivisionRegistrationController
::ToggleModel()
{

  this->State.ModelIsVisible = !this->State.ModelIsVisible;
  this->UpdateCellData();
  this->UpdateModelTransform();
  this->Render();

}

void
SubdivisionRegistrationController
::ToggleModelWires()
{
  this->State.ModelWiresAreVisible = !this->State.ModelWiresAreVisible;
  this->window.SetModelWiresVisible(this->State.ModelWiresAreVisible);
  this->Render();
}

void
SubdivisionRegistrationController
::ToggleModelSurface()
{
  this->State.ModelSurfaceIsVisible = !this->State.ModelSurfaceIsVisible;
  this->window.SetModelSurfaceVisible(this->State.ModelSurfaceIsVisible);
  this->Render();
}

void
SubdivisionRegistrationController
::ToggleColorbar()
{
  this->State.ColorbarIsVisible = !this->State.ColorbarIsVisible;
  this->window.SetColorbarVisible(this->State.ColorbarIsVisible);
  this->Render();
}

void
SubdivisionRegistrationController
::ToggleResiduals()
{
  this->State.ResidualsAreVisible = !this->State.ResidualsAreVisible;

  if (this->State.ResidualsAreVisible)
    {
    const auto file
      = this->FileTree.ResidualMeshPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses,
          this->GetCurrentFrame()
                                                  );
    this->window.UpdateResidualsSource( file );
    }

  this->window.SetResidualsVisible(this->State.ResidualsAreVisible);
  this->Render();
}

//
// Setup
//

void
SubdivisionRegistrationController
::SetupImage()
{
  const auto imageFileName = this->FileTree.ImagePathForFrame(this->GetCurrentFrame());
  const auto iren = this->ui->imageWindow->interactor();

  this->window.SetupImageData(imageFileName);
  this->window.SetupImageInformation();
  this->window.SetupImagePlane(iren);
}

void
SubdivisionRegistrationController
::SetupImageVolume()
{
  this->window.SetupImageVolume();
  this->window.SetImageVolumeVisible( this->State.ImageVolumeIsVisible );
  this->Render();
}

void
SubdivisionRegistrationController
::SetupImagePlanes()
{

  // GUARD: Make sure data exists
  if (!this->State.PlaneDataExists)
    {
    this->State.ImagePlanesAreVisible = false;
    this->State.ImagePlanesHaveBeenSetup = false;
    return;
    }

  // GUARD: Don't setup again.
  if (this->State.ImagePlanesHaveBeenSetup)
    return;

  if (this->State.ImagePlaneHasBeenSet)
    {
    this->State.planeWidgetState.RestoreState(this->window.planeWidget);
    this->window.PlaneWidgetDidMove();
    }

//  this->window.UpdatePlanesPlacement();

  this->State.ImagePlanesHaveBeenSetup = true;

  this->window.SetImagePlanesVisible( this->State.ImagePlanesAreVisible );

  this->Render();

}

void
SubdivisionRegistrationController
::SetupReslicePlanes()
{

  if (!this->State.ImagePlanesHaveBeenSetup)
    {   
    std::cerr << "Image planes have not been setup." << std::endl;
    std::cerr << "Returning." << std::endl;
    return;
    }   

  this->window.SetupReslicePlanesSA(this->State.ReslicePlanesSANumber);
  this->window.SetupReslicePlanesLA(this->State.ReslicePlanesLANumber);

  this->UpdateReslicePlanesPlacement();

  this->window.SetReslicePlanesVisible( this->State.ReslicePlanesAreVisible );

  this->Render();

}

void
SubdivisionRegistrationController
::SetupCandidates()
{

  // GUARD: Make sure data exists.
  if (!this->State.CandidateDataExists)
    {
    this->State.CandidatesAreVisible = false;
    this->State.CandidatesHaveBeenSetup = false;
    return;
    }

  // GUARD: Don't setup again.
  if (this->State.CandidatesHaveBeenSetup)
    return;

  const auto frame = this->GetCurrentFrame();
  const auto fileName = this->FileTree.CandidatePathForFrame(frame);

  this->window.SetupCandidates(fileName);

  this->State.CandidatesHaveBeenSetup = true;

  this->window.SetCandidatesVisible(this->State.CandidatesAreVisible);
  this->Render();

}

void
SubdivisionRegistrationController
::SetupModel()
{

  // GUARD: Don't setup again.
  if (this->State.ModelHasBeenSetup)
    return;

  if (this->State.GetCurrentState() == dv::State::REGISTERED)
    {

    this->window.SetupModel(
      this->FileTree.RegisteredModelPathForPassAndFrame(
        this->State.NumberOfRegistrationPasses,
        this->GetCurrentFrame()
                                                       )
                           );

    }
  else if (this->State.GetCurrentState() == dv::State::ORIENTATION_CAPTURED)
    {

    if (!this->State.InitialModelDataExists)
      {
      std::cout << "Generating initial model" << std::endl;
      this->GenerateInitialModel();
      }

    // Setup.
    this->window.SetupModel( this->FileTree.InitialModel.c_str() );

    }

  this->State.ModelHasBeenSetup = this->window.ModelHasBeenSetup;

  ///////////////
  // Residuals //
  ///////////////

  if (this->State.InitialModelDataExists && this->State.ModelHasBeenSetup)
    {
    const auto file = this->FileTree.ResidualMeshPathForPassAndFrame(
      this->State.NumberOfRegistrationPasses,
      this->GetCurrentFrame()
                                                                 );
    this->window.SetupResiduals( file );

    this->State.ResidualsAreVisible = true;
    }
    
  ////////////
  // Render //
  ////////////

  if (this->State.ModelHasBeenSetup)
    {
    this->State.ModelIsVisible = true;
    
    // Ensure visibility.
    this->UpdateModelTransform();
    this->Render();
    }

}

void
SubdivisionRegistrationController
::GenerateInitialModel()
{

  const auto inputMeshName
    = this->FileTree.CandidatePathForFrame( this->State.EDFrame );

  const auto outputMeshName
    = this->FileTree.InitialModel;

  const auto count = this->State.NumberOfFacesInDecimatedMesh;

  const auto sigma = this->State.DecimationNoiseSigma;

  dv::GenerateInitialModel(
    inputMeshName,
    outputMeshName,
    count,
    sigma
    );

}

void
SubdivisionRegistrationController
::Serialize()
{

  this->State.planeWidgetState.CaptureState(this->window.planeWidget);

  this->State.camera.CaptureState(this->window.renderer->GetActiveCamera());

    {
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    this->State.SerializeJSON(writer);
    writer.EndObject();

    std::string fileName = this->FileTree.ParametersJSON;
    std::ofstream fileStream;
    fileStream.open(fileName);
    fileStream << sb.GetString();
    fileStream.close();
    }

    {
    if (!this->State.SurfaceAreas.empty())
      {
      std::string fileName
        = this->FileTree.SurfaceAreaForPass(this->State.NumberOfRegistrationPasses - 1);
      std::ofstream fileStream;
      fileStream.open(fileName);
      this->State.SerializeSurfaceArea(fileStream, this->State.SurfaceAreas);
      fileStream.close();
      }
    }

    {
    if (!this->State.costFunctionFrames.empty())
      {
      std::string fileName
        = this->FileTree.ResidualsForPass(this->State.NumberOfRegistrationPasses - 1);
      std::ofstream fileStream;
      fileStream.open(fileName);
      this->State.SerializeResiduals(fileStream);
      fileStream.close();
      }
    }

    {
    if (this->State.RegistrationSummary != "")
      {
      std::string fileName
        = this->FileTree.RegistrationSummaryForPass(this->State.NumberOfRegistrationPasses - 1);
      std::ofstream fileStream;
      fileStream.open(fileName);
      fileStream << this->State.RegistrationSummary;
      fileStream.close();
      }
    }

    {
    if (nullptr != this->State.SegmentIDs)
      {
      std::string fileName
        = this->FileTree.SegmentationIDsForPass(this->State.NumberOfRegistrationPasses - 1);
      std::ofstream fileStream;
      fileStream.open(fileName);
      this->State.SerializeSegmentation(fileStream, this->State.SegmentIDs);
      fileStream.close();
      }
    }

}

void
SubdivisionRegistrationController
::Deserialize()
{

    {
    const auto fileName = this->FileTree.ParametersJSON;
    if (itksys::SystemTools::FileExists(fileName,true))
      {
      std::ifstream fileStream;
      fileStream.open(fileName);
      std::stringstream buffer;
      buffer << fileStream.rdbuf();
      fileStream.close();

      rapidjson::Document d;
      d.Parse(buffer.str().c_str());

      this->State.DeserializeJSON(d);
      this->State.camera.RestoreState(this->window.renderer->GetActiveCamera());
      }
    }

    {
    const auto fileName
      = this->FileTree.SurfaceAreaForPass( this->State.NumberOfRegistrationPasses - 1 );
    if (itksys::SystemTools::FileExists(fileName,true))
      {
      std::ifstream fileStream;
      fileStream.open(fileName);
      this->State.DeserializeSurfaceArea(fileStream);
      fileStream.close();
      }
    }

    {
    const auto fileName
      = this->FileTree.ResidualsForPass( this->State.NumberOfRegistrationPasses - 1 );
    if (itksys::SystemTools::FileExists(fileName,true))
      {
      std::ifstream fileStream;
      fileStream.open(fileName);
      this->State.DeserializeResiduals(fileStream);
      fileStream.close();
      }
    }

}

SubdivisionRegistrationController
::~SubdivisionRegistrationController()
{
  this->Serialize();
}

unsigned int
SubdivisionRegistrationController
::GetCurrentFrame()
{
  itkAssertOrThrowMacro(this->ui->frameSlider->value() >= 0,
                        "The value of the frame slider was negative.");
  return static_cast<unsigned int>(this->ui->frameSlider->value());
}

void
SubdivisionRegistrationController
::CalculateResidualsForPass(const unsigned int pass)
{

  std::cout << "Calculating residuals for pass " << pass << "..." << std::endl;
  auto progress = dv::Progress( this->FileTree.GetNumberOfFiles() );

  // Get the vectors
  typedef itk::PointsLocator< TMesh::PointsContainer >         TLocator;
  typedef dv::CalculateResidualMesh< TMesh, TLoopMesh, TMesh > TResidualCalculator;

  typedef itk::MeshFileReader< TMesh >     TFixedReader;
  typedef itk::MeshFileReader< TLoopMesh > TMovingReader;
  typedef itk::MeshFileWriter< TMesh >     TWriter;

  const auto initial = TLoopMesh::New();

  if (0 == pass)
    {
    const auto reader = TMovingReader::New();
    reader->SetFileName( this->FileTree.InitialModel );
    reader->Update();
    initial->Graft( reader->GetOutput() );
    initial->Setup();
    }

  for (unsigned int f = 0; f < this->FileTree.GetNumberOfFiles(); ++f)
    {

    const auto locator = TLocator::New();

      {
      // Fixed
      auto reader = TFixedReader::New();
      reader->SetFileName( this->FileTree.CandidatePathForFrame(f) );
      reader->Update();

      const auto mesh = TMesh::New();
      mesh->Graft( reader->GetOutput() );

      locator->SetPoints( mesh->GetPoints() );
      }

    locator->Initialize();

    if (0 != pass)
      {
      const auto reader = TMovingReader::New();
      const auto file
        = this->FileTree.RegisteredModelPathForPassAndFrame( pass - 1, f );
      reader->SetFileName( file );
      reader->Update();
      initial->Graft( reader->GetOutput() );
      initial->Setup();
      }

    TResidualCalculator residuals(locator, initial);

    const auto r = residuals.Calculate();

    const std::string dir = this->FileTree.ResidualsDirectory + std::to_string(pass);
    this->FileTree.CreateDirectory( dir );

      {
      const auto writer = TWriter::New();
      writer->SetInput(r);
      writer->SetFileName(this->FileTree.ResidualMeshPathForPassAndFrame(pass, f));
      writer->Update();
      }

      progress.UnitCompleted();

    }

}

void
SubdivisionRegistrationController
::Register()
{

  //////////////
  // Register //
  //////////////

  std::cout << "Performing registration." << std::endl;

  itk::TimeProbe clock;
  clock.Start();

  // Get the vectors
  typedef itk::Mesh< TReal, 3, TMeshTraits > TFixed;
  typedef itk::LoopSubdivisionSurfaceMesh< TReal, 3, TQEMeshTraits > TMoving;
  typedef itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter< TMoving, TMoving > TLoop;
  typedef itk::PointsLocator< TFixed::PointsContainer > TLocator;
  typedef dv::RegisterMeshToPointSet< TFixed, TMoving > TRegister;

  typedef itk::MeshFileReader< TFixed > TFixedReader;
  typedef itk::MeshFileReader< TMoving > TMovingReader;
  typedef itk::MeshFileWriter< TMoving > TMovingWriter;

  std::vector<TLocator::Pointer> locatorVector;
  std::vector<TMoving::Pointer> movingVector;

  std::string dirToCreate =
    this->FileTree.RegisteredModelDirectory +
    std::to_string(this->State.NumberOfRegistrationPasses);

  this->FileTree.CreateDirectory( dirToCreate );

  std::cout << "Preparing boundary candidates..." << std::endl;

  auto progress = dv::Progress( this->FileTree.GetNumberOfFiles() );

  for (unsigned int i = 0; i < this->FileTree.GetNumberOfFiles(); ++i)
    {

    // Fixed
    const auto fixedReader = TFixedReader::New();
    fixedReader->SetFileName( this->FileTree.CandidatePathForFrame(i) );
    fixedReader->Update();

    const auto locator = TLocator::New();
    locator->SetPoints( fixedReader->GetOutput()->GetPoints() );
    locator->Initialize();

    locatorVector.emplace_back(locator);

    // Moving 
    const auto movingReader = TMovingReader::New();

    if (0 == this->State.NumberOfRegistrationPasses)
      {
      movingReader->SetFileName( this->FileTree.InitialModel );
      movingReader->Update();
      movingVector.emplace_back(movingReader->GetOutput());
      }
    else
      {
      const auto file
        = this->FileTree.RegisteredModelPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses - 1, i);
      movingReader->SetFileName( file );
      const auto loop = TLoop::New();
      loop->SetInput( movingReader->GetOutput() );
      loop->Update();
      movingVector.emplace_back(loop->GetOutput());
      }

    for (auto moving : movingVector) moving->Setup();

    progress.UnitCompleted();

    }

  TRegister registerMesh(this->State.EDFrame,
                         locatorVector,
                         movingVector);
  registerMesh.RegistrationWeights = this->State.RegistrationWeights;

  registerMesh.Register();

  std::cout << "Writing registered models..." << std::endl;

  for (unsigned int i = 0; i < this->FileTree.GetNumberOfFiles(); ++i)
    {
    const auto file
      = this->FileTree.RegisteredModelPathForPassAndFrame(
        this->State.NumberOfRegistrationPasses, i);
    const auto finalWriter = TMovingWriter::New();
    finalWriter->SetInput( movingVector.at(i) );
    finalWriter->SetFileName( file );
    finalWriter->Update();

    }

  std::cout << "done." << std::endl;

  ////////////////////////////////
  // Calculate residuals/SQUEEZ //
  ////////////////////////////////

  this->State.NumberOfRegistrationPasses += 1;

  std::cout << "Calculating state variables..." << std::flush;

  this->State.RegistrationSummary = registerMesh.summaryString;
  this->State.costFunctionFrames  = registerMesh.costFunctionFrames;
  this->State.costFunctionCellIDs = registerMesh.costFunctionCellIDs;

  std::vector<TReal> residualX;
  std::vector<TReal> residualY;
  std::vector<TReal> residualZ;

  for (std::size_t i = 0; i < registerMesh.costFunctionResiduals.size();)
    {
    residualX.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualY.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualZ.emplace_back(registerMesh.costFunctionResiduals[i++]);
    }

  this->State.costFunctionResidualX = residualX;
  this->State.costFunctionResidualY = residualY;
  this->State.costFunctionResidualZ = residualZ;

  this->CalculateSurfaceAreas();
  this->CalculateResidualsForPass(this->State.NumberOfRegistrationPasses);
  this->CalculateSegmentIDs();

  std::cout << "done." << std::endl;

  clock.Stop();
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;

  this->UpdateModelTransform();

}

void
SubdivisionRegistrationController
::WriteScreenshots()
{

  std::cout << "Writing screenshots..." << std::endl;
  auto progress = dv::Progress( this->FileTree.GetNumberOfFiles() );

  for (unsigned int i = 0; i < this->FileTree.GetNumberOfFiles(); ++i)
    {
    this->ui->frameSlider->setValue( i );

    this->ui->imageWindow->renderWindow()->SetAlphaBitPlanes(1);

    const auto windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput( this->ui->imageWindow->renderWindow() );
    windowToImageFilter->SetScale(2);
    windowToImageFilter->SetInputBufferTypeToRGBA();
    windowToImageFilter->SetFixBoundary(true);
    windowToImageFilter->Update();

    const auto writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName( this->FileTree.ScreenshotPathForFrame(i).c_str() );
    writer->SetInputConnection( windowToImageFilter->GetOutputPort() );
    writer->Write();

    progress.UnitCompleted();

    }
}

void
SubdivisionRegistrationController
::UpdateCellData()
{

  // Colorbar visibility
  this->window.SetColorbarVisible(!this->ui->cellDataButtonNone->isChecked());

  // Update state and colorbar visibility based on the value of the radio button.
  if (this->ui->cellDataButtonNone->isChecked())
    {
    this->State.CellDataToDisplay = CellData::NONE;
    }
  else if (this->ui->cellDataButtonSQUEEZ->isChecked())
    {
    this->State.CellDataToDisplay = CellData::SQUEEZ;
    }
  else if (this->ui->cellDataButtonResidual->isChecked())
    {
    this->State.CellDataToDisplay = CellData::RESIDUALS;
    }
  else if (this->ui->cellDataButtonSegmentation->isChecked())
    {
    this->State.CellDataToDisplay = CellData::SEGMENTATION;
    }
  else
    {
    std::cerr << "Selection not recognized." << std::endl;
    return;
    }

  // Update Colorbar
  this->window.CB_State = this->State.ColorbarState.at(this->State.CellDataToDisplay);
  this->window.UpdateLookupTable();

  // Update the display based on the updated state.
  switch (this->State.CellDataToDisplay)
    {
    case CellData::NONE:
      {
      this->window.modelCellData = nullptr;
      return;
      }
    case CellData::RESIDUALS:
      {

      if (
          this->State.costFunctionResidualX.empty() ||
          this->State.costFunctionResidualX.size() != this->State.costFunctionResidualY.size() ||
          this->State.costFunctionResidualX.size() != this->State.costFunctionResidualZ.size()
         )
        {
        std::cout << "Residual block is not the correct size." << std::endl;
        this->window.modelCellData = nullptr;
        return;
        }

      this->window.modelCellData
        = this->State.CalculateResidualsForFrame(this->GetCurrentFrame());

      break;
      }
    case CellData::SQUEEZ:
      {

      if (this->State.NumberOfRegistrationPasses < 1)
        {
        std::cerr << "Warning: Surface areas cannot be calculated." << std::endl;
        this->window.modelCellData = nullptr;
        return;
        }

      if (this->State.SurfaceAreas.empty() ||
          this->State.SurfaceAreas.cols() != this->FileTree.GetNumberOfFiles())
        {
        this->CalculateSurfaceAreas();
        }

      this->window.modelCellData = this->State.SQUEEZ[this->GetCurrentFrame()];

      break;
      }
    case CellData::SEGMENTATION:
      {
      if (this->State.NumberOfRegistrationPasses < 1)
        {
        std::cerr << "Warning: Surface areas cannot be calculated." << std::endl;
        this->window.modelCellData = nullptr;
        return;
        }
      if (nullptr == this->State.SegmentIDs)
        {
        this->CalculateSegmentIDs();
        }
      this->window.modelCellData = this->State.SegmentIDs;

      break;
      }
    default:
      {
      std::cout << "Cell Data type not recognized." << std::endl;
      break;
      }
    }

}

std::vector<unsigned short>
SubdivisionRegistrationController
::CalculateSASegmentIDsForCellData(const std::vector<std::array<double, 3>> &centers)
{

  std::vector<unsigned short> sa_index(centers.size(), 0);

  for (size_t i = 0; i < centers.size(); ++i)
    {
    for (size_t N = 0; N < this->State.ReslicePlanesSANumber; ++N)
      {
      const auto dist =
        dv::SignedDistanceToPlane(this->window.reslicePlanesSA[N]->GetNormal(),
                                  this->window.reslicePlanesSA[N]->GetOrigin(),
                                  centers[i].data());
      if (dist > 0.0)
        {
        sa_index[i] +=1;
        }
      else
        {
        break;
        }
      }
    }

  return sa_index;

}

std::vector<unsigned short>
SubdivisionRegistrationController
::CalculateLASegmentIDsForCellData(const std::vector<std::array<double, 3>> &centers)
{

  std::vector<unsigned short> la_index(centers.size(), 0);

  for (size_t i = 0; i < centers.size(); ++i)
    {
    for (size_t p = 0; p < this->State.ReslicePlanesLANumber*2; ++p)
      {
      const size_t p_mod = p       % this->State.ReslicePlanesLANumber;
      const size_t n_mod = (p + 1) % this->State.ReslicePlanesLANumber;

      const size_t p_flp = p       / this->State.ReslicePlanesLANumber;
      const size_t n_flp = (p + 1) / this->State.ReslicePlanesLANumber;

      const auto p_dist =
        dv::SignedDistanceToPlane(this->window.reslicePlanesLA[p_mod]->GetNormal(),
                                  this->window.reslicePlanesLA[p_mod]->GetOrigin(),
                                  centers[i].data()) * std::pow(-1, p_flp);
      const auto n_dist =
        dv::SignedDistanceToPlane(this->window.reslicePlanesLA[n_mod]->GetNormal(),
                                  this->window.reslicePlanesLA[n_mod]->GetOrigin(),
                                  centers[i].data()) * std::pow(-1, n_flp);

      if (p_dist > 0.0 && n_dist < 0.0)
        {
        la_index[i] = this->State.ReslicePlanesLANumber * 2 - (p + 1);
        break;
        }
      }
    }

  return la_index;

}

vtkSmartPointer<vtkFloatArray>
SubdivisionRegistrationController
::CalculateSegmentIDsForCellData(const std::vector<std::array<double, 3>> &centers)
{

  const auto sa_index = this->CalculateSASegmentIDsForCellData(centers);
  const auto la_index = this->CalculateLASegmentIDsForCellData(centers);

  const auto cellData = vtkSmartPointer<vtkFloatArray>::New();
  cellData->SetNumberOfComponents(1);
  cellData->SetNumberOfValues( centers.size() );

  for (vtkIdType i = 0; i < cellData->GetSize(); ++i)
    {
    unsigned short cell_index =
      la_index[i] + sa_index[i] * 2 * this->State.ReslicePlanesLANumber;
    cellData->SetValue( i, cell_index );
    }

  return cellData;

}

void
SubdivisionRegistrationController
::CalculateSegmentIDs()
{

  //////////////////////////
  // For the Actual Model //
  //////////////////////////

    {
    if (!this->State.EDFrameHasBeenSet)
      {
      std::cerr << "WARNING: ED frame has not been set; returning." << std::endl;
      return;
      }
  
    const auto file = this->FileTree.RegisteredModelPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses - 1, this->State.EDFrame
                                                                       );
  
    if (!itksys::SystemTools::FileExists(file,true))
      {
      std::cerr << "WARNING: File does not exist; returning." << std::endl;
      return;
      }
  
    const auto modelReader = TVTKMeshReader::New();
    modelReader->SetFileName( file.c_str() );
    modelReader->Update();
  
    const auto modelLoop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
    modelLoop->SetInputData( modelReader->GetOutput() );
    modelLoop->SetNumberOfSubdivisions( this->State.NumberOfSubdivisions );
    modelLoop->Update();
  
    const auto centers = dv::CalculateTriangleCenters( modelLoop->GetOutput() );
  
    this->State.SegmentIDs = this->CalculateSegmentIDsForCellData(centers);
    }

}

void
SubdivisionRegistrationController
::CalculateSurfaceAreas()
{

  ///////////////////
  // Surface Areas //
  ///////////////////

    {
    std::cout << "Calculating surface areas..." << std::endl;
    auto progress = dv::Progress( this->FileTree.GetNumberOfFiles() );
  
    vnl_matrix<TReal> sa;
  
    for (unsigned int f = 0; f < this->FileTree.GetNumberOfFiles(); ++f)
      {
  
      const auto modelReader = TVTKMeshReader::New();
      modelReader->SetFileName(
        this->FileTree.RegisteredModelPathForPassAndFrame(
          this->State.NumberOfRegistrationPasses - 1, f ).c_str()
                              );
      modelReader->Update();
  
      const auto modelLoop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
      modelLoop->SetInputData( modelReader->GetOutput() );
      modelLoop->SetNumberOfSubdivisions( this->State.NumberOfSubdivisions );
      modelLoop->Update();
  
      if (0 == f)
        {
        sa.set_size(modelLoop->GetOutput()->GetNumberOfCells(),
                    this->FileTree.GetNumberOfFiles());
        sa.fill(-1.0);
        }
  
      sa.set_column(f, dv::CalculateSurfaceAreas(modelLoop->GetOutput()).data());
  
      progress.UnitCompleted();
  
      }
  
    this->State.SurfaceAreas = sa;
  
    for (unsigned int f = 0; f < this->FileTree.GetNumberOfFiles(); ++f)
      {
      this->State.SQUEEZ[f] = this->State.CalculateSQUEEZ(f);
      }
    }

}

}

#endif

