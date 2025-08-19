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

namespace sissr {

// CONSTRUCTOR
Controller
::Controller(int argc, char** argv) :
  DirStructure(argv[1], argv[2]) {

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
  switch (this->State.CellDataToDisplay)
    {
    case CellData::NONE:
      this->ui->cellDataButtonNone->setChecked(true);
      break;
    case CellData::SQUEEZ:
      this->ui->cellDataButtonSQUEEZ->setChecked(true);
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

  this->ui->frameSlider->setRange(      0,this->DirStructure.GetNumberOfFiles()-1);

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
  val %= this->DirStructure.GetNumberOfFiles();
  this->ui->frameSlider->setValue( val );

}

void
Controller
::DecrementFrame()
{

  int val = this->GetCurrentFrame() - 1;
  if (val < 0) val += this->DirStructure.GetNumberOfFiles();
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
    this->window.UpdatePlanesSource(this->DirStructure.ImageDirectory.PathForFrame(value));
    }

  /**************
   * Candidates *
   **************/

  if (this->State.CandidatesAreVisible)
    {
    this->window.UpdateCandidatesSource(this->DirStructure.CandidateDirectory.PathForFrame(value));
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

  if ( !(this->DirStructure.InitialModelDataExists() && this->State.ModelHasBeenSetup) )
    {
    return;
    }

  if ( !this->State.ModelIsVisible )
    {
    this->window.SetModelVisible(false);
    this->window.SetColorbarVisible(false);
    return;
    }


  if (0 == this->DirStructure.NumberOfRegistrationPasses())
    {
    this->window.SetModelVisible(true);
    this->window.SetColorbarVisible(false);
    return;
    }

  // Model Surface File
  if (this->State.ModelSurfaceIsVisible || this->State.ModelWiresAreVisible) {
    const auto p = this->DirStructure.NumberOfRegistrationPasses();
    const auto f = this->GetCurrentFrame();
    const auto file
      = this->DirStructure.RegisteredModelPathForPassAndFrame(p-1,f);
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
  auto progress = dv::Progress( this->DirStructure.GetNumberOfFiles() );

  itk::TimeProbe clock;
  clock.Start();

  using TClean = itk::CleanSegmentationImageFilter<TImage>;
  using TCuberille = itk::CuberilleImageToMeshFilter<TImage, TQEMesh>;

  for (size_t file = 0; file < this->DirStructure.GetNumberOfFiles(); ++file) {

    const auto input = this->DirStructure.SegmentationDirectory.PathForFrame(file);
    const auto output = this->DirStructure.CandidateDirectory.PathForFrame(file);

    const auto reader = TImageReader::New();
    reader->SetFileName(input);

    const auto clean = TClean::New();
    clean->SetInput( reader->GetOutput() );

    const auto cuberille = TCuberille::New();
    cuberille->SetInput(clean->GetOutput());
    cuberille->GenerateTriangleFacesOff();
    // cuberille->RemoveProblematicPixelsOn(); // Method not available in current ITK version
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
  reader->SetFileName(this->DirStructure.SegmentationDirectory.PathForFrame(this->State.InitialModelParams.GetFrame()));

  const auto model = TModel::New();
  model->SetInput(reader->GetOutput());
  model->SetNumberOfCellsInDecimatedMesh(this->State.InitialModelParams.GetFaces());
  model->SetMeshNoiseSigma(this->State.InitialModelParams.GetSigma());
  model->SetLVClosingRadius(this->State.InitialModelParams.GetLVClosingRadius());
  model->SetGeneralClosingRadius(this->State.InitialModelParams.GetGeneralClosingRadius());
  model->SetPreserveEdges(this->State.InitialModelParams.GetPreserveEdges());
  model->SetDecimationTechnique(this->State.InitialModelParams.GetDecimationTechnique());

  const auto writer = TWriter::New();
  writer->SetInput(model->GetOutput());
  writer->SetFileName(this->DirStructure.InitialModel);
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
  if (!this->DirStructure.ImageDirectory.DataExists())
    {
    this->State.ImagePlanesAreVisible = false;
    this->State.ImagePlanesHaveBeenSetup = false;
    return;
    }

  this->State.ImagePlanesAreVisible = !this->State.ImagePlanesAreVisible;

  if (this->State.ImagePlanesAreVisible)
    {
    const auto file = this->DirStructure.ImageDirectory.PathForFrame( this->GetCurrentFrame() );
    this->window.UpdatePlanesSource( file );
    }

  this->window.SetImagePlanesVisible(this->State.ImagePlanesAreVisible);
  this->Render();

}

void
Controller
::ToggleCandidates() {

  if (!this->DirStructure.CandidateDirectory.DataExists()) {
    std::cerr << "Candidate data doesn't exist." << std::endl;
    return;
    }

  this->State.CandidatesAreVisible = !this->State.CandidatesAreVisible;

  if (this->State.CandidatesAreVisible) {
    const auto file = this->DirStructure.CandidateDirectory.PathForFrame(this->GetCurrentFrame());
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


//
// Setup
//

void
Controller
::SetupImage()
{
  // GUARD: Make sure data exists
  if (!this->DirStructure.ImageDirectory.DataExists())
    {
    this->State.ImagePlanesAreVisible = false;
    this->State.ImagePlanesHaveBeenSetup = false;
    return;
    }

  const auto imageFileName = this->DirStructure.ImageDirectory.PathForFrame(this->GetCurrentFrame());
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
//  if (!this->DirStructure.ImageDirectory.DataExists())
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
  if (!this->DirStructure.CandidateDirectory.DataExists()) {
    this->State.CandidatesAreVisible = false;
    this->State.CandidatesHaveBeenSetup = false;
    return;
  }

  // GUARD: Don't setup again.
  if (this->State.CandidatesHaveBeenSetup) {
    return;
  }

  const auto frame = this->GetCurrentFrame();
  const auto fileName = this->DirStructure.CandidateDirectory.PathForFrame(frame);

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

  if (this->DirStructure.NumberOfRegistrationPasses() > 0) {

    const auto p = this->DirStructure.NumberOfRegistrationPasses();
    const auto f = this->GetCurrentFrame();
    const auto file = this->DirStructure.RegisteredModelPathForPassAndFrame(p-1,f);
    this->window.SetupModel(file);

    }
  else if (this->DirStructure.InitialModelDataExists()) {

    // Setup.
    this->window.SetupModel( this->DirStructure.InitialModel.c_str() );

  } else {

    std::cerr << "Initial model does not exist." << std::endl;
    return;

  }

  this->State.ModelHasBeenSetup = this->window.ModelHasBeenSetup;


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
  this->State.camera.SerializeJSON(this->DirStructure.CameraParametersJSON);
  this->State.InitialModelParams.SerializeJSON(this->DirStructure.InitialModelParametersJSON);
  this->State.SerializeJSON(this->DirStructure.ParametersJSON);

    {
    if (!this->State.SurfaceAreas.empty())
      {
      std::string fileName
        = this->DirStructure.SurfaceAreaForPass(this->DirStructure.NumberOfRegistrationPasses() - 1);
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
        = this->DirStructure.ResidualsForPass(this->DirStructure.NumberOfRegistrationPasses() - 1);
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
        = this->DirStructure.RegistrationSummaryForPass(this->DirStructure.NumberOfRegistrationPasses() - 1);
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
    const auto fileName = this->DirStructure.ParametersJSON;
    if (std::filesystem::exists(fileName)) {
      this->State.DeserializeJSON(fileName);
    }
  }
  {
    const auto fileName = this->DirStructure.InitialModelParametersJSON;
    if (std::filesystem::exists(fileName)) {
      this->State.InitialModelParams.DeserializeJSON(fileName);
    }
  }
  {
    const auto fileName = this->DirStructure.CameraParametersJSON;
    if (std::filesystem::exists(fileName)) {
      this->State.camera.DeserializeJSON(fileName);
      this->State.camera.RestoreState(this->window.renderer->GetActiveCamera());
    }
  }

  {
  const auto fileName
    = this->DirStructure.SurfaceAreaForPass( this->DirStructure.NumberOfRegistrationPasses() - 1 );
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
    = this->DirStructure.ResidualsForPass( this->DirStructure.NumberOfRegistrationPasses() - 1 );
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
::Register()
{

  //////////////
  // Register //
  //////////////

  std::cout << "Performing registration." << std::endl;

  itk::TimeProbe clock;
  clock.Start();

  // Get the vectors
  using TLoop = itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter<TLoopMesh, TLoopMesh>;
  using TLocator = itk::PointsLocator<TMesh::PointsContainer>;
  using TRegister = RegisterMeshToPointSet<TMesh, TLoopMesh>;
  using TMovingWriter = itk::MeshFileWriter<TLoopMesh>;

  std::string dirToCreate =
    this->DirStructure.RegisteredModelDirectory +
    std::to_string(this->DirStructure.NumberOfRegistrationPasses());

  std::filesystem::create_directories( dirToCreate );

  std::cout << "Preparing boundary candidates..." << std::endl;

  auto progress = dv::Progress( this->DirStructure.GetNumberOfFiles() );

  // Fixed

  std::vector<TMesh::Pointer> fixedVector;

  for (unsigned int i = 0; i < this->DirStructure.GetNumberOfFiles(); ++i) {

    const auto reader = TMeshReader::New();
    const auto filename = this->DirStructure.CandidateDirectory.PathForFrame(i);
    reader->SetFileName(filename);

    try {
      reader->Update();
      auto mesh = reader->GetOutput();

      // Check if cell data exists, if not create it with all cells labeled as 1
      if (!mesh->GetCellData() || mesh->GetCellData()->Size() == 0) {
        auto cellData = TMesh::CellDataContainer::New();
        cellData->Reserve(mesh->GetNumberOfCells());

        // Assign label 1 to all cells
        for (auto cellIt = mesh->GetCells()->Begin(); cellIt != mesh->GetCells()->End(); ++cellIt) {
          cellData->SetElement(cellIt.Index(), 1.0f);
        }

        mesh->SetCellData(cellData);
        std::cout << "Added default cell data (label=1) to " << mesh->GetNumberOfCells() << " cells in " << filename << std::endl;
      }

      fixedVector.emplace_back(mesh);
    } catch (const itk::ExceptionObject& e) {
      std::cerr << "ERROR: Failed to read mesh file: " << filename << std::endl;
      std::cerr << "ITK Exception: " << e.GetDescription() << std::endl;
      throw;
    }

  }

  // Moving

  std::vector<TLoopMesh::Pointer> movingVector;

  for (unsigned int i = 0; i < this->DirStructure.GetNumberOfFiles(); ++i) {

    if (0 == this->DirStructure.NumberOfRegistrationPasses()) {
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(this->DirStructure.InitialModel);
      reader->Update();
      reader->GetOutput()->SetSurfaceSampleDensity(this->State.RegistrationSamplingDensity);
      reader->GetOutput()->Setup();
      movingVector.emplace_back(reader->GetOutput());
    } else {
      const auto p = this->DirStructure.NumberOfRegistrationPasses();
      const auto file
        = this->DirStructure.RegisteredModelPathForPassAndFrame(p-1, i);
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(file);

      const auto loop = TLoop::New();
      loop->SetInput(reader->GetOutput());
      loop->Update();
      loop->GetOutput()->Setup();
      movingVector.emplace_back(loop->GetOutput());
      }

    // Ensure the last added moving mesh has cell data
    auto& mesh = movingVector.back();
    if (!mesh->GetCellData() || mesh->GetCellData()->Size() == 0) {
      auto cellData = TLoopMesh::CellDataContainer::New();
      cellData->Reserve(mesh->GetNumberOfCells());

      // Assign label 1 to all cells
      for (auto cellIt = mesh->GetCells()->Begin(); cellIt != mesh->GetCells()->End(); ++cellIt) {
        cellData->SetElement(cellIt.Index(), 1.0f);
      }

      mesh->SetCellData(cellData);
      std::cout << "Added default cell data (label=1) to " << mesh->GetNumberOfCells() << " cells in moving mesh" << std::endl;
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

  for (unsigned int i = 0; i < this->DirStructure.GetNumberOfFiles(); ++i) {
    const auto p = this->DirStructure.NumberOfRegistrationPasses();
    const auto file
      = this->DirStructure.RegisteredModelPathForPassAndFrame(p, i);
    // Remove point data (normals) to avoid OBJ writer error
    auto mesh = movingVector.at(i);
    if (mesh->GetPointData() && mesh->GetPointData()->Size() > 0) {
      mesh->GetPointData()->Initialize();
      std::cout << "Cleared point data (normals) from mesh before writing to " << file << std::endl;
    }

    const auto finalWriter = TMovingWriter::New();
    finalWriter->SetInput( mesh );
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
  auto progress = dv::Progress( this->DirStructure.GetNumberOfFiles() );

  this->DirStructure.AddScreenshotDirectory();

  for (unsigned int i = 0; i < this->DirStructure.GetNumberOfFiles(); ++i)
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
    writer->SetFileName( this->DirStructure.ScreenshotPathForFrame(i).c_str() );
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

      if (this->DirStructure.NumberOfRegistrationPasses() < 1)
        {
        std::cerr << "Warning: Surface areas cannot be calculated." << std::endl;
        this->window.modelCellData = nullptr;
        return;
        }

      if (this->State.SurfaceAreas.empty() ||
          this->State.SurfaceAreas.cols() != this->DirStructure.GetNumberOfFiles())
        {
        this->CalculateSurfaceAreas();
        }

      this->window.modelCellData = this->State.SQUEEZ[this->GetCurrentFrame()];
      const auto cbrange = this->State.ColorbarSettings.at(this->State.CellDataToDisplay);
      const auto lut = dv::LUT::SQUEEZ(cbrange.Min, cbrange.Max);
      this->window.UpdateLookupTable(lut);

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
    auto progress = dv::Progress( this->DirStructure.GetNumberOfFiles() );

    vnl_matrix<double> sa;

    for (unsigned int f = 0; f < this->DirStructure.GetNumberOfFiles(); ++f) {

      const auto p = this->DirStructure.NumberOfRegistrationPasses();
      const auto modelReader = TVTKMeshReader::New();
      const auto file = this->DirStructure.RegisteredModelPathForPassAndFrame(p-1, f);
      modelReader->SetFileName(file.c_str());
      modelReader->Update();

      const auto modelLoop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
      modelLoop->SetInputData( modelReader->GetOutput() );
      modelLoop->SetNumberOfSubdivisions( this->State.NumberOfSubdivisions );
      modelLoop->Update();

      if (0 == f)
        {
        sa.set_size(modelLoop->GetOutput()->GetNumberOfCells(),
                    this->DirStructure.GetNumberOfFiles());
        sa.fill(-1.0);
        }

      sa.set_column(f, dv::CalculateSurfaceAreas(modelLoop->GetOutput()).data());

      progress.UnitCompleted();

      }

    this->State.SurfaceAreas = sa;

    for (unsigned int f = 0; f < this->DirStructure.GetNumberOfFiles(); ++f)
      {
      this->State.SQUEEZ[f] = this->State.CalculateSQUEEZ(f);
      }
    }

  }

} // namespace sissr
