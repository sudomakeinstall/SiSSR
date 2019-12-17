#ifndef itkDirectoryStructure_cxx
#define itkDirectoryStructure_cxx

#include <dvDirectoryStructure.h>

#include <itkFileTools.h>
#include <vector>
#include <string>

namespace dv
{

DirectoryStructure
::DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory) :
IptDirectory(_IptDirectory.back() == '/' ? _IptDirectory : _IptDirectory + '/'),
OptDirectory(_OptDirectory.back() == '/' ? _OptDirectory : _OptDirectory + '/')
{

  // Create directories
  this->CreateDirectory(this->CandidateDirectory);
  this->CreateDirectory(this->InitialModelDirectory);
  this->CreateDirectory(this->RegisteredModelDirectory);
  this->CreateDirectory(this->ResidualsDirectory);
  this->CreateDirectory(this->SerializationDirectory);
  this->CreateDirectory(this->ScreenshotDirectory);

  this->DetermineNumberOfFiles();

};
  
unsigned int
DirectoryStructure
::GetNumberOfFiles() const
{
  return this->NumberOfFiles;
}

void
DirectoryStructure
::DetermineNumberOfFiles()
{
  this->NumberOfFiles = 0;
  while (
    itksys::SystemTools::FileExists(
      this->ImageDirectory + std::to_string(NumberOfFiles) + this->ImageSuffix,true)
        ) ++NumberOfFiles;
  itkAssertOrThrowMacro(NumberOfFiles > 0, "At least one image must be supplied.");
}

void
DirectoryStructure
::CreateDirectory(const std::string &Directory)
{
  itk::FileTools::CreateDirectory(Directory.c_str());
}

std::string
DirectoryStructure
::SegmentationIDsForPass(const unsigned int p) const
{
  return this->SerializationDirectory + "segmentation_" + std::to_string(p) + ".txt";
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
    if (!itksys::SystemTools::FileExists(this->CandidatePathForFrame(f),true)) {
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
      if (!itksys::SystemTools::FileExists(file,true)) {
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
    if (!itksys::SystemTools::FileExists(file,true)) {
      return false;
    }
  }
  return true;
}

}
#endif
