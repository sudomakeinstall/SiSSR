#include <sissrParameters.h>

// STD
#include <iostream>
#include <fstream>
#include <sstream>

// Helper functions for JSON handling
namespace {
  void check_and_set_uint(const rapidjson::Document& d, unsigned int& var, const std::string& key) {
    if (d.HasMember(key.c_str()) && d[key.c_str()].IsUint()) {
      var = d[key.c_str()].GetUint();
    }
  }
  void check_and_set_bool(const rapidjson::Document& d, bool& var, const std::string& key) {
    if (d.HasMember(key.c_str()) && d[key.c_str()].IsBool()) {
      var = d[key.c_str()].GetBool();
    }
  }
  void check_and_set_double(const rapidjson::Document& d, double& var, const std::string& key) {
    if (d.HasMember(key.c_str()) && d[key.c_str()].IsDouble()) {
      var = d[key.c_str()].GetDouble();
    }
  }
}

namespace sissr {

Parameters::Parameters() :
  RegistrationSamplingDensity(2)
{}

void
Parameters::SerializeJSON(const std::string &fileName) {

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  writer.StartObject();
  this->SerializeJSON(writer);
  writer.EndObject();

  std::ofstream fileStream;
  fileStream.open(fileName);
  fileStream << sb.GetString();
  fileStream.close();
}

void
Parameters::SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer)
{
  writer.Key("CurrentFrame");
  writer.Uint(this->CurrentFrame);

  writer.Key("RegistrationUseLabels");
  writer.Bool(this->RegistrationUseLabels);
  writer.Key("RegistrationWeights.Primary");
  writer.Double(this->RegistrationWeights.Primary);
  writer.Key("RegistrationWeights.EdgeWeight");
  writer.Double(this->RegistrationWeights.EdgeWeight);
  writer.Key("RegistrationWeights.Velocity");
  writer.Double(this->RegistrationWeights.Velocity);
  writer.Key("RegistrationWeights.Acceleration");
  writer.Double(this->RegistrationWeights.Acceleration);
  writer.Key("RegistrationWeights.ThinPlate");
  writer.Double(this->RegistrationWeights.ThinPlate);
  writer.Key("RegistrationWeights.TriangleAspectRatio");
  writer.Double(this->RegistrationWeights.TriangleAspectRatio);
  writer.Key("RegistrationWeights.EdgeLength");
  writer.Double(this->RegistrationWeights.EdgeLength);
  writer.Key("RegistrationSamplingDensity");
  writer.Uint(this->RegistrationSamplingDensity);

  writer.Key("MaximumNumberOfIterations");
  writer.Int(this->MaximumNumberOfIterations);
  writer.Key("MaximumSolverTimeInSeconds");
  writer.Int(this->MaximumSolverTimeInSeconds);
  writer.Key("FunctionTolerance");
  writer.Double(this->FunctionTolerance);
  writer.Key("ParameterTolerance");
  writer.Double(this->ParameterTolerance);
  writer.Key("DynamicSparsity");
  writer.Bool(this->DynamicSparsity);

  writer.Key("NumberOfSubdivisions");
  writer.Uint(this->NumberOfSubdivisions);
}

void
Parameters::DeserializeJSON(const std::string &fileName)
{
  std::ifstream fileStream;
  fileStream.open(fileName);
  std::stringstream buffer;
  buffer << fileStream.rdbuf();
  fileStream.close();

  rapidjson::Document d;
  d.Parse(buffer.str().c_str());

  this->DeserializeJSON(d);
}

void
Parameters::DeserializeJSON(const rapidjson::Document &d)
{
  check_and_set_uint(d, this->CurrentFrame, "CurrentFrame");

  check_and_set_bool(d, this->RegistrationUseLabels, "RegistrationUseLabels");
  check_and_set_double(d, this->RegistrationWeights.Primary, "RegistrationWeights.Primary");
  check_and_set_double(d, this->RegistrationWeights.EdgeWeight, "RegistrationWeights.EdgeWeight");
  check_and_set_double(d, this->RegistrationWeights.Velocity, "RegistrationWeights.Velocity");
  check_and_set_double(d, this->RegistrationWeights.Acceleration, "RegistrationWeights.Acceleration");
  check_and_set_double(d, this->RegistrationWeights.ThinPlate, "RegistrationWeights.ThinPlate");
  check_and_set_double(d, this->RegistrationWeights.TriangleAspectRatio, "RegistrationWeights.TriangleAspectRatio");
  check_and_set_double(d, this->RegistrationWeights.EdgeLength, "RegistrationWeights.EdgeLength");
  check_and_set_uint(d, this->RegistrationSamplingDensity, "RegistrationSamplingDensity");

  // Solver parameters - need separate helper for int
  if (d.HasMember("MaximumNumberOfIterations") && d["MaximumNumberOfIterations"].IsInt()) {
    this->MaximumNumberOfIterations = d["MaximumNumberOfIterations"].GetInt();
  }
  if (d.HasMember("MaximumSolverTimeInSeconds") && d["MaximumSolverTimeInSeconds"].IsInt()) {
    this->MaximumSolverTimeInSeconds = d["MaximumSolverTimeInSeconds"].GetInt();
  }
  check_and_set_double(d, this->FunctionTolerance, "FunctionTolerance");
  check_and_set_double(d, this->ParameterTolerance, "ParameterTolerance");
  check_and_set_bool(d, this->DynamicSparsity, "DynamicSparsity");

  check_and_set_uint(d, this->NumberOfSubdivisions, "NumberOfSubdivisions");
}

void
Parameters::SerializeSurfaceArea(std::ofstream& s, const vnl_matrix<double> &areas)
{
  s << areas.rows() << std::endl;
  s << areas.cols() << std::endl;

  for (unsigned int r = 0; r < areas.rows(); ++r) {
    for (unsigned int c = 0; c < areas.cols(); ++c) {
      s << areas(r,c);
      if (c < areas.cols() - 1) s << " ";
    }
    s << std::endl;
  }
}

void
Parameters::DeserializeSurfaceArea(std::ifstream& s)
{
  size_t rows, cols;
  s >> rows >> cols;

  SurfaceAreas.set_size(rows, cols);

  for (size_t r = 0; r < rows; ++r) {
    for (size_t c = 0; c < cols; ++c) {
      s >> SurfaceAreas(r,c);
    }
  }
}

void
Parameters::SerializeResiduals(std::ofstream &s)
{
  s << costFunctionCellIDs.size() << std::endl;
  
  for (size_t i = 0; i < costFunctionCellIDs.size(); ++i) {
    s << costFunctionCellIDs[i] << " ";
    s << costFunctionFrames[i] << " ";
    s << costFunctionResidualX[i] << " ";
    s << costFunctionResidualY[i] << " ";
    s << costFunctionResidualZ[i] << std::endl;
  }
}

void
Parameters::DeserializeResiduals(std::ifstream& s)
{
  size_t size;
  s >> size;

  costFunctionCellIDs.resize(size);
  costFunctionFrames.resize(size);
  costFunctionResidualX.resize(size);
  costFunctionResidualY.resize(size);
  costFunctionResidualZ.resize(size);

  for (size_t i = 0; i < size; ++i) {
    s >> costFunctionCellIDs[i];
    s >> costFunctionFrames[i];
    s >> costFunctionResidualX[i];
    s >> costFunctionResidualY[i];
    s >> costFunctionResidualZ[i];
  }
}

} // namespace sissr
