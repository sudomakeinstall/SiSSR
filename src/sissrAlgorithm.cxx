#include <sissrAlgorithm.h>

// STD
#include <filesystem>
#include <iostream>

// ITK
#include <itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkTimeProbe.h>
#include <itkIterativeTriangleCellSubdivisionQuadEdgeMeshFilter.h>
#include <itkQuadEdgeMesh.h>


// SiSSR
#include <sissrRegisterMeshToPointSet.h>

namespace sissr {

// CONSTRUCTOR
Algorithm::Algorithm(const std::string& candidateDir, const std::string& initialModel, const std::string& outputDir) :
  dirStructure(candidateDir, initialModel, outputDir) {

  // Determine current state from serialized parameters
  if (std::filesystem::exists(dirStructure.ParametersJSON)) {
    parameters.DeserializeJSON(dirStructure.ParametersJSON);
  }
}

// DESTRUCTOR
Algorithm::~Algorithm() {
  // Save parameters on destruction
  parameters.SerializeJSON(dirStructure.ParametersJSON);
}


void
Algorithm::Register()
{
  std::cout << "Performing registration." << std::endl;

  itk::TimeProbe clock;
  clock.Start();

  // Get the vectors
  using TLoop = itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter<TLoopMesh, TLoopMesh>;
  using TLocator = itk::PointsLocator<TMesh::PointsContainer>;
  using TRegister = RegisterMeshToPointSet<TMesh, TLoopMesh>;
  using TMovingWriter = itk::MeshFileWriter<TLoopMesh>;

  std::string dirToCreate =
    dirStructure.RegisteredModelDirectory +
    std::to_string(dirStructure.NumberOfRegistrationPasses());

  std::filesystem::create_directories(dirToCreate);

  std::cout << "Loading fixed meshes..." << std::endl;


  // Fixed
  std::vector<TMesh::Pointer> fixedVector;

  for (unsigned int i = 0; i < dirStructure.GetNumberOfFiles(); ++i) {

    const auto reader = TMeshReader::New();
    const auto filename = dirStructure.CandidateDirectory.PathForFrame(i);
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

  for (unsigned int i = 0; i < dirStructure.GetNumberOfFiles(); ++i) {

    if (0 == dirStructure.NumberOfRegistrationPasses()) {
      const auto reader = TLoopMeshReader::New();
      reader->SetFileName(dirStructure.InitialModel);
      reader->Update();
      reader->GetOutput()->SetSurfaceSampleDensity(parameters.RegistrationSamplingDensity);
      reader->GetOutput()->Setup();
      movingVector.emplace_back(reader->GetOutput());
    } else {
      const auto p = dirStructure.NumberOfRegistrationPasses();
      const auto file = dirStructure.RegisteredModelPathForPassAndFrame(p-1, i);
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

  }

  TRegister registerMesh(
      fixedVector,
      movingVector,
      parameters.RegistrationUseLabels);

  registerMesh.RegistrationWeights = parameters.RegistrationWeights;
  registerMesh.MaximumNumberOfIterations = parameters.MaximumNumberOfIterations;
  registerMesh.MaximumSolverTimeInSeconds = parameters.MaximumSolverTimeInSeconds;
  registerMesh.FunctionTolerance = parameters.FunctionTolerance;
  registerMesh.ParameterTolerance = parameters.ParameterTolerance;
  registerMesh.DynamicSparsity = parameters.DynamicSparsity;

  registerMesh.Register();

  std::cout << "Writing registered models..." << std::endl;

  for (unsigned int i = 0; i < dirStructure.GetNumberOfFiles(); ++i) {
    const auto p = dirStructure.NumberOfRegistrationPasses();
    const auto file = dirStructure.RegisteredModelPathForPassAndFrame(p, i);
    // Remove point data (normals) to avoid OBJ writer error
    auto mesh = movingVector.at(i);
    if (mesh->GetPointData() && mesh->GetPointData()->Size() > 0) {
      mesh->GetPointData()->Initialize();
      std::cout << "Cleared point data (normals) from mesh before writing to " << file << std::endl;
    }

    const auto finalWriter = TMovingWriter::New();
    finalWriter->SetInput(mesh);
    finalWriter->SetFileName(file);
    finalWriter->Update();
  }

  std::cout << "done." << std::endl;

  parameters.RegistrationSummary = registerMesh.summaryString;
  parameters.costFunctionFrames  = registerMesh.costFunctionFrames;
  parameters.costFunctionCellIDs = registerMesh.costFunctionCellIDs;

  std::vector<double> residualX;
  std::vector<double> residualY;
  std::vector<double> residualZ;

  for (size_t i = 0; i < registerMesh.costFunctionResiduals.size();) {
    residualX.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualY.emplace_back(registerMesh.costFunctionResiduals[i++]);
    residualZ.emplace_back(registerMesh.costFunctionResiduals[i++]);
  }

  parameters.costFunctionResidualX = residualX;
  parameters.costFunctionResidualY = residualY;
  parameters.costFunctionResidualZ = residualZ;

  std::cout << "done." << std::endl;

  clock.Stop();
  std::cout << "Time elapsed: " << clock.GetTotal() << std::endl;
}

} // namespace sissr
