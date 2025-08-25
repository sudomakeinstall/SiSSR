#ifndef sissr_Parameters_h
#define sissr_Parameters_h

// STD
#include <vector>
#include <map>

// RapidJSON
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

// VNL
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>

// SiSSR
#include <sissrLossScaleFactors.h>

namespace sissr {

class Parameters
{

public:

  Parameters();

  ////////////////
  // Parameters //
  ////////////////


  bool RegistrationUseLabels = true;
  LossScaleFactors RegistrationWeights;
  unsigned int RegistrationSamplingDensity = 2;

  // Registration solver parameters
  int MaximumNumberOfIterations = 500;
  int MaximumSolverTimeInSeconds = 3600; // 1 hour
  double FunctionTolerance = 1e-6;
  double ParameterTolerance = 1e-8;
  bool DynamicSparsity = false;

  unsigned int CurrentFrame = 0;

  // Serialization
  void SerializeJSON(const std::string &fileName);
  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer);

  void DeserializeJSON(const std::string &fileName);
  void DeserializeJSON(const rapidjson::Document &d);

  // Surface area calculation
  void SerializeSurfaceArea(std::ofstream& s, const vnl_matrix<double> &areas);
  void DeserializeSurfaceArea(std::ifstream& s);

  vnl_matrix<double> SurfaceAreas;
  unsigned int SurfacePointDensity = 2;

  // Registration results
  std::vector<unsigned int> costFunctionCellIDs;
  std::vector<unsigned int> costFunctionFrames;
  std::vector<double>       costFunctionResidualX;
  std::vector<double>       costFunctionResidualY;
  std::vector<double>       costFunctionResidualZ;

  void SerializeResiduals(std::ofstream &s);
  void DeserializeResiduals(std::ifstream& s);

  std::string RegistrationSummary;
  unsigned int NumberOfSubdivisions = 3;

}; // end class

} // end namespace

#endif
