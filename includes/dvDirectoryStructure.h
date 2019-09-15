#ifndef itkDirectoryStructure_h
#define itkDirectoryStructure_h

#include <itkFileTools.h>
#include <vector>
#include <string>

namespace dv
{
class DirectoryStructure
{
public:

  // Constructor
  DirectoryStructure(const std::string _IptDirectory,
                     const std::string _OptDirectory);
  
  const std::string IptDirectory;
  const std::string OptDirectory;
  
  const std::string ImageDirectory           = this->IptDirectory + "img-nii/";
  const std::string SegmentationDirectory    = this->IptDirectory + "seg-nii/";

  const std::string CandidateDirectory       = this->OptDirectory + "candidates/";
  const std::string InitialModelDirectory    = this->OptDirectory + "initial_models/";
  const std::string RegisteredModelDirectory = this->OptDirectory + "registered_models/";
  const std::string ResidualsDirectory       = this->OptDirectory + "residuals_models/";
  const std::string SerializationDirectory   = this->OptDirectory + "serialization/";
  const std::string ScreenshotDirectory      = this->OptDirectory + "screenshots/";

  const std::string ImageSuffix = ".nii.gz";
  const std::string MeshSuffix = ".obj";
  const std::string ResidualSuffix = ".vtk";
  const std::string ScreenshotSuffix = ".png";

  const std::string InitialModel           = this->InitialModelDirectory + "initial_model" + MeshSuffix;
  const std::string InitialSmoothedModel   = this->InitialModelDirectory + "initial_smoothed_model" + MeshSuffix;
  const std::string InitialSubdividedModel = this->InitialModelDirectory + "initial_subdivided_model" + MeshSuffix;

  const std::string SegmentationIDs = this->SerializationDirectory + "segmentation.txt";
  const std::string ParametersJSON = this->SerializationDirectory + "parameters.json";

  std::string SegmentationIDsForPass(const unsigned int p) const;
  std::string SurfaceAreaForPass(const unsigned int p) const;

  std::string ResidualsForPass(const unsigned int p) const;
  std::string RegistrationSummaryForPass(const unsigned int p) const;

  std::string ImagePathForFrame(const unsigned int f) const
    { return this->ImageDirectory + std::to_string(f) + this->ImageSuffix; }
  std::string SegmentationPathForFrame(const unsigned int f) const
    { return this->SegmentationDirectory + std::to_string(f) + this->ImageSuffix; }

  std::string CandidatePathForFrame(const unsigned int f) const
    { return this->CandidateDirectory + std::to_string(f) + this->MeshSuffix; }
  std::string InitialModelPathForFrame(const unsigned int f) const;
  std::string ScreenshotPathForFrame(const unsigned int f) const
    { return this->ScreenshotDirectory + std::to_string(f) + this->ScreenshotSuffix; }

  std::string RegisteredModelPathForPassAndFrame(const unsigned int p, const unsigned int f) const;
  std::string ResidualMeshPathForPassAndFrame(const unsigned int p, const unsigned int f) const;
  unsigned int GetNumberOfFiles() const;

  void CreateDirectory(const std::string &Directory);

  bool CandidateDataExists() const
    {
    for (size_t f = 0; f < this->GetNumberOfFiles(); ++f)
      {
      if (!itksys::SystemTools::FileExists(this->CandidatePathForFrame(f),true))
        {
        return false;
        }
      }
    return true;
    }

  unsigned int NumberOfRegistrationPasses() const
    {
    unsigned int NumberOfRegistrationPasses = 0;
    while (true)
      {
      bool AllFilesFound = true;
      for (unsigned int f = 0; f < this->GetNumberOfFiles(); ++f)
        {
        const auto file
          = this->RegisteredModelPathForPassAndFrame(NumberOfRegistrationPasses, f);
        if (!itksys::SystemTools::FileExists(file,true))
          {
          AllFilesFound = false;
          break;
          }
        }
      if (AllFilesFound)
        {
        ++NumberOfRegistrationPasses;
        }
      else
        {
        break;
        }
      }
    return NumberOfRegistrationPasses;
    }

  bool ResidualMeshDataExistsForPass(const unsigned int p) const
    {
    for (unsigned int f = 0; f < this->GetNumberOfFiles(); ++f)
      {
      const auto file = this->ResidualMeshPathForPassAndFrame(p, f);
      if (!itksys::SystemTools::FileExists(file,true))
        {
        return false;
        }
      }
    return true;
    }

private:

  unsigned int NumberOfFiles = 0;
  void DetermineNumberOfFiles();

};
}
#endif
