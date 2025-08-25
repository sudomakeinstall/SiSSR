// STD
#include <filesystem>

// ITK
#include <itkMacro.h>

// Custom
#include <sissrUtils.h>
#include <sissrDirectoryStructure.h>

namespace sissr {

DirectoryStructure
::DirectoryStructure(const std::string _CandidateDirectory,
                     const std::string _InitialModelPath,
                     const std::string _OptDirectory) :
  OptDirectory(sissr::ensureTrailingSlash(_OptDirectory)),
  CandidateDirectory(sissr::ensureTrailingSlash(_CandidateDirectory), ".obj"),
  InitialModel(_InitialModelPath)
{

  // Create directories
  namespace fs = std::filesystem;
  fs::create_directories(this->RegisteredModelDirectory);
  fs::create_directories(this->SerializationDirectory);

};

size_t
DirectoryStructure
::GetNumberOfFiles() const
{
  return this->CandidateDirectory.NumberOfFiles;
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
