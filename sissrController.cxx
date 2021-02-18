#include <sissrController.h>

// STD
#include <filesystem>
#include <iostream>
#include <thread>

// VTK
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkCamera.h>
#include <vtkRendererCollection.h>

// ITK
#include <itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkTimeProbe.h>
#include <itkIterativeTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkQuadEdgeMesh.h>
#include <itkQuadEdgeMeshDecimationCriteria.h>
#include <itkQuadricDecimationQuadEdgeMeshFilter.h>
#include <itkSquaredEdgeLengthDecimationQuadEdgeMeshFilter.h>
#include <itkSmoothingQuadEdgeMeshFilter.h>
#include <itkQuadEdgeMeshParamMatrixCoefficients.h>
#include <itkAdditiveGaussianNoiseQuadEdgeMeshFilter.h>

// DVCppUtils
#include <dvProgress.h>
#include <dvSignedDistanceToPlane.h>
#include <dvCyclicMean.h>
#include <itkGenerateInitialModelImageToMeshFilter.h>
#include <dvCalculateSurfaceAreas.h>
#include <dvCalculateTriangleCenters.h>
#include <dvGetTimeString.h>
#include <dvGetLookupTable.h>

// SiSSR
#include <sissrRegisterMeshToPointSet.h>
#include <sissrCalculateResidualMesh.h>

namespace sissr {

// CONSTRUCTOR
Controller
::Controller(int argc, char** argv) :
  DirectoryStructure(argv[1], argv[2]) {

  // Determine current state
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

  // Setup radio buttons
  this->ui->cellDataButtonNone->setChecked(false);
  this->ui->cellDataButtonSQUEEZ->setChecked(false);
  this->ui->cellDataButtonResidual->setChecked(false);
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
    }

  // Setup IMAGE pipeline
  this->SetupImage();
  this->SetupImagePlanes();
  this->SetupCandidates();
  this->SetupModel();

  this->ui->frameSlider->setValue(this->State.CurrentFrame);

  this->Render();

};

void
Controller
::SetupSliderRanges()
{

  this->ui->frameSlider->setRange(      0,this->DirectoryStructure.GetNumberOfFiles()-1);

}

void
Controller
::SetupValueLabels()
{

  this->ui->frameValue->setText(
    QString::fromStdString(std::to_string(this->GetCurrentFrame())) );

  if (this->State.EDFrameHasBeenSet)
    this->ui->edValue->setText(
      QString::fromStdString(std::to_string(this->State.EDFrame))
                              );
}

void
Controller
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

  connect(this->ui->togglePlanesButton,
          SIGNAL(pressed()), this, SLOT(ToggleImagePlanes()));

  connect(this->ui->toggleCandidatesButton,
          SIGNAL(pressed()), this, SLOT(ToggleCandidates()));

  connect(this->ui->toggleModelButton,
          SIGNAL(pressed()), this, SLOT(ToggleModel()));

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

}

void
Controller
::EDButtonPressed()
{

  this->State.EDFrame = this->GetCurrentFrame();
  this->ui->edValue->setText(
    QString::fromStdString(std::to_string(this->State.EDFrame)));

  this->State.EDFrameHasBeenSet = true;
  this->UpdateCurrentIndex();

}

void
Controller
::JumpToEDButtonPressed()
{
  if (this->State.EDFrameHasBeenSet)
    {
    this->ui->frameSlider->setValue(this->State.EDFrame);
    }
}

void
Controller
::UpdateCurrentIndex()
{

  const int index = this->State.CurrentFrame;

  if (index != this->ui->toolBox->currentIndex())
    {
    this->ui->toolBox->setCurrentIndex(index);
    }

}

void
Controller
::IncrementFrame()
{

  int val = this->GetCurrentFrame() + 1;
  val %= this->DirectoryStructure.GetNumberOfFiles();
  this->ui->frameSlider->setValue( val );

}

void
Controller
::DecrementFrame()
{

  int val = this->GetCurrentFrame() - 1;
  if (val < 0) val += this->DirectoryStructure.GetNumberOfFiles();
  this->ui->frameSlider->setValue( val );

}

void
Controller
::CurrentPageChanged(int index)
{

//  enum State requestedState = static_cast<enum State>(index);
//
//  if (requestedState <= this->State.GetCurrentState())
//    return;
//
//  QMessageBox alert;
//  alert.setText("This tab's analysis must be completed before proceeding.");
//  alert.exec();
//
//  this->ui->toolBox->setCurrentIndex( static_cast<int>(this->State.GetCurrentState()));

}

void
Controller
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
    this->window.UpdatePlanesSource(this->DirectoryStructure.ImageDirectory.PathForFrame(value));
    }

  /**************
   * Candidates *
   **************/

  if (this->State.CandidatesAreVisible)
    {
    this->window.UpdateCandidatesSource(this->DirectoryStructure.CandidateDirectory.PathForFrame(value));
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
Controller
::UpdateAnnotations() {
  this->window.SetPhaseAnnotation(std::to_string(this->GetCurrentFrame()));
}

void
Controller
::UpdateModelTransform()
{

  if ( !(this->DirectoryStructure.InitialModelDataExists() && this->State.ModelHasBeenSetup) )
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
      = this->DirectoryStructure.ResidualMeshPathForPassAndFrame(
          this->DirectoryStructure.NumberOfRegistrationPasses(),
          this->GetCurrentFrame()
                                                  );
    this->window.UpdateResidualsSource( file );
    }

  if (0 == this->DirectoryStructure.NumberOfRegistrationPasses())
    {
    this->window.SetModelVisible(true);
    this->window.SetColorbarVisible(false);
    return;
    }

  // Model Surface File
  if (this->State.ModelSurfaceIsVisible || this->State.ModelWiresAreVisible) {
    const auto p = this->DirectoryStructure.NumberOfRegistrationPasses();
    const auto f = this->GetCurrentFrame();
    const auto file
      = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(p-1,f);
    this->window.UpdateModelSource( file );
    }

  this->window.SetModelVisible(true);
  this->window.SetColorbarVisible(CellData::NONE != this->State.CellDataToDisplay &&
                                  this->State.ColorbarIsVisible);

}

void
Controller
::ResetCamera()
{
  this->ui->imageWindow->renderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
}

void
Controller
::Render()
{
  this->ui->imageWindow->renderWindow()->Render();
}

void
Controller
::CalculateBoundaryCandidates()
{

  std::cout << "Calculating boundary candidates..." << std::endl;
  auto progress = dv::Progress( this->DirectoryStructure.GetNumberOfFiles() );

  itk::TimeProbe clock;
  clock.Start();

  using TClean = itk::CleanSegmentationImageFilter<TImage>;
  using TCuberille = itk::CuberilleImageToMeshFilter<TImage, TQEMesh>;

  for (size_t file = 0; file < this->DirectoryStructure.GetNumberOfFiles(); ++file) {

    const auto input = this->DirectoryStructure.SegmentationDirectory.PathForFrame(file);
    const auto output = this->DirectoryStructure.CandidateDirectory.PathForFrame(file);

    const auto reader = TImageReader::New();
    reader->SetFileName(input);

    const auto clean = TClean::New();
    clean->SetInput( reader->GetOutput() );

    const auto cuberille = TCuberille::New();
    cuberille->SetInput(clean->GetOutput());
    cuberille->GenerateTriangleFacesOff();
    cuberille->RemoveProblematicPixelsOn();
    cuberille->SavePixelAsCellDataOn();

    const auto writer = TQEMeshWriter::New();
    writer->SetInput(cuberille->GetOutput());
    writer->SetFileName(output);
    writer->Update();

    progress.UnitCompleted();

  }

  clock.Stop();

  std::cout << "done." << std::endl;
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;

  this->SetupCandidates();

}

void
Controller
::GenerateInitialModel() {

  std::cout << "Generating initial model...";

  itk::TimeProbe clock;
  clock.Start();

  using TModel = itk::GenerateInitialModelImageToMeshFilter<TImage,TQEMesh>;
  using TWriter = itk::MeshFileWriter<TQEMesh>;

  const auto reader = TImageReader::New();
  reader->SetFileName(this->DirectoryStructure.InitialModelSegmentation);

  const auto model = TModel::New();
  model->SetInput(reader->GetOutput());
  model->SetNumberOfCellsInDecimatedMesh(this->State.InitialModelNumberOfFaces);
  model->SetMeshNoiseSigma(this->State.InitialModelSigma);
  model->SetLVClosingRadius(this->State.InitialModelLVClosingRadius);
  model->SetGeneralClosingRadius(this->State.InitialModelGeneralClosingRadius);
  model->SetPreserveEdges(this->State.InitialModelPreserveEdges);

  const auto writer = TWriter::New();
  writer->SetInput(model->GetOutput());
  writer->SetFileName(this->DirectoryStructure.InitialModel);
  writer->Update();

  clock.Stop();

  std::cout << "done." << std::endl;
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;

}

void
Controller
::ToggleImagePlanes()
{

  // GUARD: Make sure data exists
  if (!this->DirectoryStructure.ImageDirectory.DataExists())
    {
    this->State.ImagePlanesAreVisible = false;
    this->State.ImagePlanesHaveBeenSetup = false;
    return;
    }

  this->State.ImagePlanesAreVisible = !this->State.ImagePlanesAreVisible;

  if (this->State.ImagePlanesAreVisible)
    {
    const auto file = this->DirectoryStructure.ImageDirectory.PathForFrame( this->GetCurrentFrame() );
    this->window.UpdatePlanesSource( file );
    }

  this->window.SetImagePlanesVisible(this->State.ImagePlanesAreVisible);
  this->Render();

}

void
Controller
::ToggleCandidates() {

  if (!this->DirectoryStructure.CandidateDirectory.DataExists()) {
    std::cerr << "Candidate data doesn't exist." << std::endl;
    return;
    }

  this->State.CandidatesAreVisible = !this->State.CandidatesAreVisible;

  if (this->State.CandidatesAreVisible) {
    const auto file = this->DirectoryStructure.CandidateDirectory.PathForFrame(this->GetCurrentFrame());
    this->window.UpdateCandidatesSource( file );
    }

  this->window.SetCandidatesVisible(this->State.CandidatesAreVisible);
  this->Render();

}

void
Controller
::ToggleModel()
{

  this->State.ModelIsVisible = !this->State.ModelIsVisible;
  this->UpdateCellData();
  this->UpdateModelTransform();
  this->Render();

}

void
Controller
::ToggleModelWires()
{
  this->State.ModelWiresAreVisible = !this->State.ModelWiresAreVisible;
  this->window.SetModelWiresVisible(this->State.ModelWiresAreVisible);
  this->Render();
}

void
Controller
::ToggleModelSurface()
{
  this->State.ModelSurfaceIsVisible = !this->State.ModelSurfaceIsVisible;
  this->window.SetModelSurfaceVisible(this->State.ModelSurfaceIsVisible);
  this->Render();
}

void
Controller
::ToggleColorbar()
{
  this->State.ColorbarIsVisible = !this->State.ColorbarIsVisible;
  this->window.SetColorbarVisible(this->State.ColorbarIsVisible);
  this->Render();
}

void
Controller
::ToggleResiduals()
{
  this->State.ResidualsAreVisible = !this->State.ResidualsAreVisible;

  if (this->State.ResidualsAreVisible)
    {
    const auto file
      = this->DirectoryStructure.ResidualMeshPathForPassAndFrame(
          this->DirectoryStructure.NumberOfRegistrationPasses(),
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
Controller
::SetupImage()
{
  // GUARD: Make sure data exists
  if (!this->DirectoryStructure.ImageDirectory.DataExists())
    {
    this->State.ImagePlanesAreVisible = false;
    this->State.ImagePlanesHaveBeenSetup = false;
    return;
    }

  const auto imageFileName = this->DirectoryStructure.ImageDirectory.PathForFrame(this->GetCurrentFrame());
  const auto iren = this->ui->imageWindow->interactor();

  this->window.SetupImageData(imageFileName);
  this->window.SetupImageInformation();
  this->window.SetupImagePlane(iren);
}

void
Controller
::SetupImagePlanes()
{

//  // GUARD: Make sure data exists
//  if (!this->DirectoryStructure.ImageDirectory.DataExists())
//    {
//    this->State.ImagePlanesAreVisible = false;
//    this->State.ImagePlanesHaveBeenSetup = false;
//    return;
//    }
//
//  // GUARD: Don't setup again.
//  if (this->State.ImagePlanesHaveBeenSetup)
//    return;
//
//  if (this->State.ImagePlaneHasBeenSet)
//    {
//    this->State.planeWidgetState.RestoreState(this->window.planeWidget);
//    this->window.PlaneWidgetDidMove();
//    }
//
////  this->window.UpdatePlanesPlacement();
//
//  this->State.ImagePlanesHaveBeenSetup = true;
//
//  this->window.SetImagePlanesVisible( this->State.ImagePlanesAreVisible );
//
//  this->Render();

}

void
Controller
::SetupCandidates() {

  // GUARD: Make sure data exists.
  if (!this->DirectoryStructure.CandidateDirectory.DataExists()) {
    this->State.CandidatesAreVisible = false;
    this->State.CandidatesHaveBeenSetup = false;
    return;
  }

  // GUARD: Don't setup again.
  if (this->State.CandidatesHaveBeenSetup) {
    return;
  }

  const auto frame = this->GetCurrentFrame();
  const auto fileName = this->DirectoryStructure.CandidateDirectory.PathForFrame(frame);

  this->window.SetupCandidates(fileName);

  this->State.CandidatesHaveBeenSetup = true;

  this->window.SetCandidatesVisible(this->State.CandidatesAreVisible);
  this->Render();

}

void
Controller
::SetupModel()
{

  // GUARD: Don't setup again.
  if (this->State.ModelHasBeenSetup)
    return;

  if (this->DirectoryStructure.NumberOfRegistrationPasses() > 0) {

    const auto p = this->DirectoryStructure.NumberOfRegistrationPasses();
    const auto f = this->GetCurrentFrame();
    const auto file = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(p-1,f);
    this->window.SetupModel(file);

    }
  else if (this->DirectoryStructure.InitialModelDataExists()) {

    // Setup.
    this->window.SetupModel( this->DirectoryStructure.InitialModel.c_str() );

  } else {

    std::cerr << "Initial model does not exist." << std::endl;
    return;

  }

  this->State.ModelHasBeenSetup = this->window.ModelHasBeenSetup;

  ///////////////
  // Residuals //
  ///////////////

  if (this->DirectoryStructure.InitialModelDataExists() && this->State.ModelHasBeenSetup)
    {
    const auto file = this->DirectoryStructure.ResidualMeshPathForPassAndFrame(
      this->DirectoryStructure.NumberOfRegistrationPasses(),
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
Controller
::Serialize()
{

  if (nullptr != this->window.planeWidget) {
    this->State.planeWidgetState.Capture(this->window.planeWidget);
  }

  this->State.camera.CaptureState(this->window.renderer->GetActiveCamera());

    {
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    writer.StartObject();
    this->State.SerializeJSON(writer);
    writer.EndObject();

    std::string fileName = this->DirectoryStructure.ParametersJSON;
    std::ofstream fileStream;
    fileStream.open(fileName);
    fileStream << sb.GetString();
    fileStream.close();
    }

    {
    if (!this->State.SurfaceAreas.empty())
      {
      std::string fileName
        = this->DirectoryStructure.SurfaceAreaForPass(this->DirectoryStructure.NumberOfRegistrationPasses() - 1);
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
        = this->DirectoryStructure.ResidualsForPass(this->DirectoryStructure.NumberOfRegistrationPasses() - 1);
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
        = this->DirectoryStructure.RegistrationSummaryForPass(this->DirectoryStructure.NumberOfRegistrationPasses() - 1);
      std::ofstream fileStream;
      fileStream.open(fileName);
      fileStream << this->State.RegistrationSummary;
      fileStream.close();
      }
    }

}

void
Controller
::Deserialize()
{

    {
    const auto fileName = this->DirectoryStructure.ParametersJSON;
    if (std::filesystem::exists(fileName))
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
      = this->DirectoryStructure.SurfaceAreaForPass( this->DirectoryStructure.NumberOfRegistrationPasses() - 1 );
    if (std::filesystem::exists(fileName))
      {
      std::ifstream fileStream;
      fileStream.open(fileName);
      this->State.DeserializeSurfaceArea(fileStream);
      fileStream.close();
      }
    }

    {
    const auto fileName
      = this->DirectoryStructure.ResidualsForPass( this->DirectoryStructure.NumberOfRegistrationPasses() - 1 );
    if (std::filesystem::exists(fileName))
      {
      std::ifstream fileStream;
      fileStream.open(fileName);
      this->State.DeserializeResiduals(fileStream);
      fileStream.close();
      }
    }

}

Controller
::~Controller()
{
  this->Serialize();
}

unsigned int
Controller
::GetCurrentFrame()
{
  itkAssertOrThrowMacro(this->ui->frameSlider->value() >= 0,
                        "The value of the frame slider was negative.");
  return static_cast<unsigned int>(this->ui->frameSlider->value());
}

void
Controller
::CalculateResidualsForPass(const unsigned int pass) {

  std::cout << "Calculating residuals for pass " << pass << "..." << std::endl;
  auto progress = dv::Progress( this->DirectoryStructure.GetNumberOfFiles() );

  // Get the vectors
  typedef CalculateResidualMesh< TMesh, TLoopMesh, TMesh > TResidualCalculator;

  const auto initial = TLoopMesh::New();

  if (0 == pass) {
    const auto reader = TLoopMeshReader::New();
    reader->SetFileName(this->DirectoryStructure.InitialModel);
    reader->Update();
    initial->Graft(reader->GetOutput());
    initial->Setup();
  }

  for (unsigned int f = 0; f < this->DirectoryStructure.GetNumberOfFiles(); ++f) {

      const auto target = TMesh::New();

      {
        const auto reader = TMeshReader::New();
        reader->SetFileName(this->DirectoryStructure.CandidateDirectory.PathForFrame(f));
        reader->Update();
        target->Graft(reader->GetOutput());
      }

    if (0 != pass) {
      const auto file
        = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(pass-1,f);
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(file);
      reader->Update();
      initial->Graft(reader->GetOutput());
      initial->Setup();
    }

    TResidualCalculator residuals(target, initial);

    const auto r = residuals.Calculate();

    const std::string dir = this->DirectoryStructure.ResidualsDirectory + std::to_string(pass);
    std::filesystem::create_directories(dir);

      {
      const auto writer = TMeshWriter::New();
      writer->SetInput(r);
      writer->SetFileName(this->DirectoryStructure.ResidualMeshPathForPassAndFrame(pass, f));
      writer->Update();
      }

      progress.UnitCompleted();

    }

}

void
Controller
::CalculateResiduals() {

  for (size_t p = 0; p <= this->DirectoryStructure.NumberOfRegistrationPasses(); ++p) {
    if (!this->DirectoryStructure.ResidualMeshDataExistsForPass(p)) {
      this->CalculateResidualsForPass(p);
      }
    }

}

void
Controller
::Register()
{

  //////////////
  // Register //
  //////////////

  std::cout << "Performing registration." << std::endl;

  itk::TimeProbe clock;
  clock.Start();

  // Get the vectors
  typedef itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter< TLoopMesh, TLoopMesh > TLoop;
  typedef itk::PointsLocator< TMesh::PointsContainer > TLocator;
  typedef RegisterMeshToPointSet< TMesh, TLoopMesh > TRegister;
  typedef itk::MeshFileWriter< TLoopMesh > TMovingWriter;

  std::string dirToCreate =
    this->DirectoryStructure.RegisteredModelDirectory +
    std::to_string(this->DirectoryStructure.NumberOfRegistrationPasses());

  std::filesystem::create_directories( dirToCreate );

  std::cout << "Preparing boundary candidates..." << std::endl;

  auto progress = dv::Progress( this->DirectoryStructure.GetNumberOfFiles() );

  // Fixed

  std::vector<TMesh::Pointer> fixedVector;

  for (unsigned int i = 0; i < this->DirectoryStructure.GetNumberOfFiles(); ++i) {

    const auto reader = TMeshReader::New();
    reader->SetFileName(this->DirectoryStructure.CandidateDirectory.PathForFrame(i));
    reader->Update();

    fixedVector.emplace_back(reader->GetOutput());

  }

  // Moving

  std::vector<TLoopMesh::Pointer> movingVector;

  for (unsigned int i = 0; i < this->DirectoryStructure.GetNumberOfFiles(); ++i) {

    if (0 == this->DirectoryStructure.NumberOfRegistrationPasses()) {
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(this->DirectoryStructure.InitialModel);
      reader->Update();
      reader->GetOutput()->Setup();
      movingVector.emplace_back(reader->GetOutput());
    } else {
      const auto p = this->DirectoryStructure.NumberOfRegistrationPasses();
      const auto file
        = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(p-1, i);
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(file);

      const auto loop = TLoop::New();
      loop->SetInput(reader->GetOutput());
      loop->Update();
      loop->GetOutput()->Setup();
      movingVector.emplace_back(loop->GetOutput());
      }

    progress.UnitCompleted();

  }

  TRegister registerMesh(
      this->State.EDFrame,
      fixedVector,
      movingVector,
      this->State.RegistrationUseLabels);

  registerMesh.RegistrationWeights = this->State.RegistrationWeights;

  registerMesh.Register();

  std::cout << "Writing registered models..." << std::endl;

  for (unsigned int i = 0; i < this->DirectoryStructure.GetNumberOfFiles(); ++i) {
    const auto p = this->DirectoryStructure.NumberOfRegistrationPasses();
    const auto file
      = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(p, i);
    const auto finalWriter = TMovingWriter::New();
    finalWriter->SetInput( movingVector.at(i) );
    finalWriter->SetFileName( file );
    finalWriter->Update();
    }

  std::cout << "done." << std::endl;

  ////////////////////////////////
  // Calculate residuals/SQUEEZ //
  ////////////////////////////////


  this->State.RegistrationSummary = registerMesh.summaryString;
  this->State.costFunctionFrames  = registerMesh.costFunctionFrames;
  this->State.costFunctionCellIDs = registerMesh.costFunctionCellIDs;

  std::vector<double> residualX;
  std::vector<double> residualY;
  std::vector<double> residualZ;

  for (size_t i = 0; i < registerMesh.costFunctionResiduals.size();)
    {
    residualX.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualY.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualZ.emplace_back(registerMesh.costFunctionResiduals[i++]);
    }

  this->State.costFunctionResidualX = residualX;
  this->State.costFunctionResidualY = residualY;
  this->State.costFunctionResidualZ = residualZ;

  this->CalculateSurfaceAreas();
  this->CalculateResidualsForPass(this->DirectoryStructure.NumberOfRegistrationPasses());

  std::cout << "done." << std::endl;

  clock.Stop();
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;

  this->UpdateModelTransform();

}

void
Controller
::WriteScreenshots()
{

  std::cout << "Writing screenshots..." << std::endl;
  auto progress = dv::Progress( this->DirectoryStructure.GetNumberOfFiles() );

  this->DirectoryStructure.AddScreenshotDirectory();

  for (unsigned int i = 0; i < this->DirectoryStructure.GetNumberOfFiles(); ++i)
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
    writer->SetFileName( this->DirectoryStructure.ScreenshotPathForFrame(i).c_str() );
    writer->SetInputConnection( windowToImageFilter->GetOutputPort() );
    writer->Write();

    progress.UnitCompleted();

    }
}

void
Controller
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
  else
    {
    std::cerr << "Selection not recognized." << std::endl;
    return;
    }

  // Update the display based on the updated state.
  switch (this->State.CellDataToDisplay)
    {
    case CellData::NONE:
      {
      this->window.modelCellData = nullptr;
      this->window.UpdateLookupTable(dv::LUT::Rainbow());
      return;
      }
    case CellData::SQUEEZ:
      {

      if (this->DirectoryStructure.NumberOfRegistrationPasses() < 1)
        {
        std::cerr << "Warning: Surface areas cannot be calculated." << std::endl;
        this->window.modelCellData = nullptr;
        return;
        }

      if (this->State.SurfaceAreas.empty() ||
          this->State.SurfaceAreas.cols() != this->DirectoryStructure.GetNumberOfFiles())
        {
        this->CalculateSurfaceAreas();
        }

      this->window.modelCellData = this->State.SQUEEZ[this->GetCurrentFrame()];
      const auto cbrange = this->State.ColorbarState.at(this->State.CellDataToDisplay);
      const auto lut = dv::LUT::SQUEEZ(cbrange.Min, cbrange.Max);
      this->window.UpdateLookupTable(lut);

      break;
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
    default:
      {
      std::cout << "Cell Data type not recognized." << std::endl;
      break;
      }
    }

}

void
Controller
::CalculateSurfaceAreas()
{

  ///////////////////
  // Surface Areas //
  ///////////////////

    {
    std::cout << "Calculating surface areas..." << std::endl;
    auto progress = dv::Progress( this->DirectoryStructure.GetNumberOfFiles() );
  
    vnl_matrix<double> sa;
  
    for (unsigned int f = 0; f < this->DirectoryStructure.GetNumberOfFiles(); ++f) {
  
      const auto p = this->DirectoryStructure.NumberOfRegistrationPasses();
      const auto modelReader = TVTKMeshReader::New();
      const auto file = this->DirectoryStructure.RegisteredModelPathForPassAndFrame(p-1, f);
      modelReader->SetFileName(file.c_str());
      modelReader->Update();
  
      const auto modelLoop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
      modelLoop->SetInputData( modelReader->GetOutput() );
      modelLoop->SetNumberOfSubdivisions( this->State.NumberOfSubdivisions );
      modelLoop->Update();
  
      if (0 == f)
        {
        sa.set_size(modelLoop->GetOutput()->GetNumberOfCells(),
                    this->DirectoryStructure.GetNumberOfFiles());
        sa.fill(-1.0);
        }
  
      sa.set_column(f, dv::CalculateSurfaceAreas(modelLoop->GetOutput()).data());
  
      progress.UnitCompleted();
  
      }
  
    this->State.SurfaceAreas = sa;
  
    for (unsigned int f = 0; f < this->DirectoryStructure.GetNumberOfFiles(); ++f)
      {
      this->State.SQUEEZ[f] = this->State.CalculateSQUEEZ(f);
      }
    }

  }

} // namespace sissr
