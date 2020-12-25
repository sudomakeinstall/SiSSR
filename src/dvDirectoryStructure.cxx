// STD
#include <filesystem>

// ITK
#include <itkMacro.h>

// Custom
#include <dvDirectoryStructure.h>

namespace dv
{

DirectoryStructure
::DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory) :
IptDirectory(_IptDirectory.back() == '/' ? _IptDirectory : _IptDirectory + '/'),
OptDirectory(_OptDirectory.back() == '/' ? _OptDirectory : _OptDirectory + '/')
{

  // Create directories
  std::filesystem::create_directory(this->CandidateDirectory);
  std::filesystem::create_directory(this->InitialModelDirectory);
  std::filesystem::create_directory(this->RegisteredModelDirectory);
  std::filesystem::create_directory(this->ResidualsDirectory);
  std::filesystem::create_directory(this->SerializationDirectory);
  std::filesystem::create_directory(this->ScreenshotDirectory);

  this->DetermineNumberOfFiles();

};
  
void
DirectoryStructure
::DetermineNumberOfFiles()
{
  this->NumberOfFiles = 0;
  while (
    std::filesystem::exists(
      this->SegmentationDirectory + std::to_string(NumberOfFiles) + this->ImageSuffix)
        ) ++NumberOfFiles;
  itkAssertOrThrowMacro(NumberOfFiles > 0, "At least one image must be supplied.");
}

unsigned int
DirectoryStructure
::GetNumberOfFiles() const
{
  return this->NumberOfFiles;
}

std::string
DirectoryStructure
::SurfaceAreaForPass(const unsigned int p) const
{
  return this->SerializationDirectory + "surface_area_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::ResidualsForPass(const unsigned int p) const
{
  return this->SerializationDirectory + "residuals_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::RegistrationSummaryForPass(const unsigned int p) const
{
  return this->SerializationDirectory + "summary_" + std::to_string(p) + ".txt";
}

std::string
DirectoryStructure
::RegisteredModelPathForPassAndFrame(const unsigned int p, const unsigned int f) const
{
  return this->RegisteredModelDirectory +
         std::to_string(p) + '/' +
         std::to_string(f) + this->MeshSuffix;
}

std::string
DirectoryStructure
::ResidualMeshPathForPassAndFrame(const unsigned int p, const unsigned int f) const
{
  return this->ResidualsDirectory + std::to_string(p) + "/" + std::to_string(f) + this->ResidualSuffix;
}

std::string
DirectoryStructure
::ImagePathForFrame(const unsigned int f) const {
  return this->ImageDirectory + std::to_string(f) + this->ImageSuffix;
}

std::string
DirectoryStructure
::SegmentationPathForFrame(const unsigned int f) const {
  return this->SegmentationDirectory + std::to_string(f) + this->ImageSuffix;
}

std::string
DirectoryStructure
::CandidatePathForFrame(const unsigned int f) const {
  // TODO: .csv
//  return this->CandidateDirectory + std::to_string(f) + this->MeshSuffix;
  return this->CandidateDirectory + std::to_string(f) + ".txt";
}

std::string
DirectoryStructure
::ScreenshotPathForFrame(const unsigned int f) const {
  return this->ScreenshotDirectory + std::to_string(f) + this->ScreenshotSuffix;
}

bool
DirectoryStructure
::CandidateDataExists() const {
  for (size_t f = 0; f < this->GetNumberOfFiles(); ++f) {
    if (!std::filesystem::exists(this->CandidatePathForFrame(f))) {
      return false;
    }
  }
  return true;
}

unsigned int
DirectoryStructure
::NumberOfRegistrationPasses() const {
  unsigned int NumberOfRegistrationPasses = 0;
  while (true) {
    bool AllFilesFound = true;
    for (unsigned int f = 0; f < this->GetNumberOfFiles(); ++f) {
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
::ResidualMeshDataExistsForPass(const unsigned int p) const {
  for (unsigned int f = 0; f < this->GetNumberOfFiles(); ++f) {
    const auto file = this->ResidualMeshPathForPassAndFrame(p, f);
    if (!std::filesystem::exists(file)) {
      return false;
    }
  }
  return true;
}

}
