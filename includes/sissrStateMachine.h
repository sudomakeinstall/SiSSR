#ifndef sissr_StateMachine_h
#define sissr_StateMachine_h

// VTK
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPlaneWidget.h>

// VNL
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>

// RapidJSON
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

// Ceres
#include <ceres/ceres.h>

// STD
#include <iostream>
#include <list>
#include <map>

// Custom
#include <dvCameraState.h>

// SiSSR
#include <sissrColorbarState.h>
#include <sissrPlaneWidgetState.h>
#include <sissrLossScaleFactors.h>
#include <sissrCGALDecimationTechnique.h>
#include <sissrInitialModelParameters.h>

namespace sissr {

  enum class State : std::int8_t
{
  INITIALIZED = 0,
  CANDIDATES_SELECTED = 1,
  ORIENTATION_CAPTURED = 2,
  MODEL_POSITIONED = 3,
  REGISTERED = 4
};

enum class CellData : std::int8_t
{
  NONE = 0,
  SQUEEZ = 1,
};

class StateMachine
{

public:

  StateMachine();

  ////////////////
  // Parameters //
  ////////////////

  InitialModelParameters InitialModelParams;

  unsigned int BoundaryCandidateDilationRadius;

  bool RegistrationUseLabels = true;
  LossScaleFactors RegistrationWeights;
  unsigned int RegistrationSamplingDensity;

  bool EDFrameHasBeenSet = false;
  unsigned int  EDFrame;
  bool ImagePlaneHasBeenSet = false;
  unsigned int CurrentFrame = 0;

  dv::CameraState camera;

  bool ImagePlanesAreVisible   = true;
  bool CandidatesAreVisible    = true;
  bool ModelIsVisible          = true;
  bool ModelWiresAreVisible    = true;
  bool ModelSurfaceIsVisible   = true;
  bool ColorbarIsVisible       = true;

  void SerializeJSON(const std::string &fileName);
  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer);

  void DeserializeJSON(const std::string &fileName);
  void DeserializeJSON(const rapidjson::Document &d);

  PlaneWidgetState<vtkPlaneWidget> planeWidgetState;
  bool ImagePlanesHaveBeenSetup = false;
  bool CandidatesHaveBeenSetup = false;
  bool ModelHasBeenSetup = false;

  // Cell Data
  CellData CellDataToDisplay = CellData::NONE;

  void SerializeSurfaceArea(std::ofstream& s, const vnl_matrix<double> &areas);
  void DeserializeSurfaceArea(std::ifstream& s);

  vnl_matrix<double> SurfaceAreas;
  vtkSmartPointer<vtkFloatArray> CalculateSQUEEZ(unsigned int frame);
  std::map<unsigned int, vtkSmartPointer<vtkFloatArray>> SQUEEZ;
  vtkSmartPointer<vtkFloatArray> SegmentIDs = nullptr;

  unsigned int SurfacePointDensity = 2;

  vtkSmartPointer<vtkFloatArray> CalculateResidualsForFrame(unsigned int frame);

  std::vector<unsigned int> costFunctionCellIDs;
  std::vector<unsigned int> costFunctionFrames;
  std::vector<double>       costFunctionResidualX;
  std::vector<double>       costFunctionResidualY;
  std::vector<double>       costFunctionResidualZ;

  void SerializeResiduals(std::ofstream &s);
  void DeserializeResiduals(std::ifstream& s);

  void SerializeSegmentation(std::ofstream& s, vtkFloatArray* ids) const;
  void DeserializeSegmentation(std::ifstream& s);

  const std::map<CellData, ColorbarState> ColorbarSettings =
    {
      { CellData::NONE, {"", 0.0, 0.0} },
      { CellData::SQUEEZ, {"SQUEEZ", 0.7, 1.3} },
    };

  std::string RegistrationSummary;
  unsigned int NumberOfSubdivisions = 3;

}; // end class

} // end namespace

#endif
