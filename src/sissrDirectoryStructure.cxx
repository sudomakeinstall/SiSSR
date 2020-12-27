// STD
#include <filesystem>

// ITK
#include <itkMacro.h>

// Custom
#include <dvStringOperations.h>

// STD
#include <sissrDirectoryStructure.h>

namespace sissr {

DirectoryStructure
::DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory) :
  IptDirectory(dv::AppendCharacterIfAbsent(_IptDirectory, '/')),
  OptDirectory(dv::AppendCharacterIfAbsent(_OptDirectory, '/')),
  SegmentationDirectory  (IptDirectory + "seg-nii/"         , ".nii.gz"),
  ImageDirectory         (IptDirectory + "img-nii/"         , ".nii.gz", GetNumberOfFiles()),
  CandidateMeshDirectory (IptDirectory + "candidate-meshes/", ".vtk"   , GetNumberOfFiles()),
  CandidatePointDirectory(IptDirectory + "candidate-points/", ".txt"   , GetNumberOfFiles())
{

  // Create directories
  namespace fs = std::filesystem;
  fs::create_directories(this->InitialModelDirectory);
  fs::create_directories(this->RegisteredModelDirectory);
  fs::create_directories(this->ResidualsDirectory);
  fs::create_directories(this->SerializationDirectory);
  fs::create_directories(this->ScreenshotDirectory);

};
  
size_t
DirectoryStructure
::GetNumberOfFiles() const
{
  return this->SegmentationDirectory.NumberOfFiles;
}

std::string
DirectoryStructure
::RegisteredModelPathForPassAndFrame(const size_t p, const size_t f) const
{
  return this->RegisteredModelDirectory +
         std::to_string(p) + '/' +
         std::to_string(f) + this->MeshSuffix;
}

/////////////////////
// Does Data Exist //
/////////////////////

std::string
DirectoryStructure
::SurfaceAreaForPass(const size_t p) const
{
  return this->SerializationDirectory + "surface_area_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::ResidualsForPass(const size_t p) const
{
  return this->SerializationDirectory + "residuals_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::RegistrationSummaryForPass(const size_t p) const
{
  return this->SerializationDirectory + "summary_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::ResidualMeshPathForPassAndFrame(const size_t p, const size_t f) const
{
  return this->ResidualsDirectory + std::to_string(p) + "/" + std::to_string(f) + this->MeshSuffix;
}

std::string
DirectoryStructure
::ScreenshotPathForFrame(const size_t f) const {
  return this->ScreenshotDirectory + std::to_string(f) + this->ScreenshotSuffix;
}

size_t
DirectoryStructure
::NumberOfRegistrationPasses() const {
  size_t NumberOfRegistrationPasses = 0;
  while (true) {
    bool AllFilesFound = true;
    for (size_t f = 0; f < this->GetNumberOfFiles(); ++f) {
      const auto file
        = this->RegisteredModelPathForPassAndFrame(NumberOfRegistrationPasses, f);
      if (!std::filesystem::exists(file)) {
        AllFilesFound = false;
        break;
        }
    }
    if (AllFilesFound) {
      ++NumberOfRegistrationPasses;
    }
    else {
      break;
    }
  }
  return NumberOfRegistrationPasses;
}

bool
DirectoryStructure
::ResidualMeshDataExistsForPass(const size_t p) const {
  for (size_t f = 0; f < this->GetNumberOfFiles(); ++f) {
    const auto file = this->ResidualMeshPathForPassAndFrame(p, f);
    if (!std::filesystem::exists(file)) {
      return false;
    }
  }
  return true;
}

} // namespace sissr
