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
  const std::string MeshSuffix = ".vtk";
  const std::string ResidualSuffix = ".vtk";
  const std::string ScreenshotSuffix = ".png";

  const std::string InitialModelSegmentation =
    this->IptDirectory +
    "initial_model_segmentation" + ImageSuffix;
  const std::string InitialModel           = this->InitialModelDirectory + "initial_model" + MeshSuffix;

  const std::string ParametersJSON = this->SerializationDirectory + "parameters.json";

  std::string SurfaceAreaForPass(const unsigned int p) const;

  std::string ResidualsForPass(const unsigned int p) const;
  std::string RegistrationSummaryForPass(const unsigned int p) const;

  std::string ImagePathForFrame(const unsigned int f) const;
  std::string SegmentationPathForFrame(const unsigned int f) const;

  std::string CandidatePathForFrame(const unsigned int f) const;
  std::string ScreenshotPathForFrame(const unsigned int f) const;

  std::string RegisteredModelPathForPassAndFrame(const unsigned int p, const unsigned int f) const;
  std::string ResidualMeshPathForPassAndFrame(const unsigned int p, const unsigned int f) const;
  unsigned int GetNumberOfFiles() const;

  void CreateDirectory(const std::string &Directory);
  bool CandidateDataExists() const;
  unsigned int NumberOfRegistrationPasses() const;
  bool ResidualMeshDataExistsForPass(const unsigned int p) const;

private:

  unsigned int NumberOfFiles = 0;
  void DetermineNumberOfFiles();
 
};
}
#endif
