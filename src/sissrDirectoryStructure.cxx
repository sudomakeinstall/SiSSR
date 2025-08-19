// STD
#include <filesystem>

// ITK
#include <itkMacro.h>

// Custom
#include <dvStringOperations.h>
#include <dvGetTimeString.h>
#include <sissrDirectoryStructure.h>

namespace sissr {

DirectoryStructure
::DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory) :
  IptDirectory(dv::AppendCharacterIfAbsent(_IptDirectory, '/')),
  OptDirectory(dv::AppendCharacterIfAbsent(_OptDirectory, '/')),
  SegmentationDirectory  (IptDirectory + "seg-nii/"      , ".nii.gz"),
  ImageDirectory         (IptDirectory + "img-nii/"         , ".nii.gz", GetNumberOfFiles()),
  CandidateDirectory (IptDirectory + "msh-vtk/", ".vtk"   , GetNumberOfFiles())
{

  // Create directories
  namespace fs = std::filesystem;
  fs::create_directories(this->InitialModelDirectory);
  fs::create_directories(this->RegisteredModelDirectory);
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

void
DirectoryStructure
::AddScreenshotDirectory() {
  const auto dirname = dv::GetTimeString() + "/";
  this->ScreenshotDirectories.push_back(dirname);
  namespace fs = std::filesystem;
  const auto full = this->ScreenshotDirectory
    + this->ScreenshotDirectories.back();
  fs::create_directories(full);
}

std::string
DirectoryStructure
::ScreenshotPathForFrame(const size_t f) {
  if (this->ScreenshotDirectories.empty()) {
    this->AddScreenshotDirectory();
  }
  const auto p = this->ScreenshotDirectory
    + this->ScreenshotDirectories.back()
    + std::to_string(f) + this->ScreenshotSuffix;
  return p;
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


} // namespace sissr
