#ifndef sissr_DirectoryStructure_h
#define sissr_DirectoryStructure_h

// STD
#include <vector>
#include <string>

// Custom
#include <sissrSequentialDirectory.h>

namespace sissr {

class DirectoryStructure
{
public:

  // Constructor
  DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory);
 
  const std::string IptDirectory;
  const std::string OptDirectory;

  const sissr::SequentialDirectory SegmentationDirectory;
  const sissr::SequentialDirectory ImageDirectory;
  const sissr::SequentialDirectory CandidateDirectory;

  const std::string InitialModelDirectory    = this->OptDirectory + "initial_models/";
  const std::string RegisteredModelDirectory = this->OptDirectory + "registered_models/";
  const std::string ResidualsDirectory       = this->OptDirectory + "residuals_models/";
  const std::string SerializationDirectory   = this->OptDirectory + "serialization/";
  const std::string ScreenshotDirectory      = this->OptDirectory + "screenshots/";

  const std::string ImageSuffix = ".nii.gz";
  const std::string MeshSuffix = ".vtk";
  const std::string PointSuffix = ".txt";
  const std::string ScreenshotSuffix = ".png";

  const std::string InitialModel = this->InitialModelDirectory + "initial_model" + MeshSuffix;

  const std::string CameraParametersJSON = this->SerializationDirectory + "camera-parameters.json";
  const std::string InitialModelParametersJSON = this->SerializationDirectory + "initial-model-parameters.json";
  const std::string ParametersJSON = this->SerializationDirectory + "parameters.json";

  std::string SurfaceAreaForPass(const size_t p) const;

  std::string ResidualsForPass(const size_t p) const;
  std::string RegistrationSummaryForPass(const size_t p) const;

  std::vector<std::string> ScreenshotDirectories;
  void AddScreenshotDirectory();
  std::string ScreenshotPathForFrame(const size_t f);

  std::string RegisteredModelPathForPassAndFrame(const size_t p, const size_t f) const;
  std::string ResidualMeshPathForPassAndFrame(const size_t p, const size_t f) const;
  size_t GetNumberOfFiles() const;

  size_t NumberOfRegistrationPasses() const;
  bool ResidualMeshDataExistsForPass(const size_t p) const;
  bool InitialModelDataExists() {
    return std::filesystem::exists(this->InitialModel);
  }

};

} // namespace sissr

#endif
