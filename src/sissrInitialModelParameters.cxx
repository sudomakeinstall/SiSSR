// STD
#include <fstream>
#include <sstream>

// Utils
#include <dvRapidJSONHelper.h>

// SiSSR
#include <sissrInitialModelParameters.h>

namespace sissr {

InitialModelParameters
::InitialModelParameters(
      unsigned int        _Faces,
      double              _Sigma,
      unsigned int        _LVClosingRadius,
      unsigned int        _GeneralClosingRadius,
      bool                _PreserveEdges,
      unsigned int        _Frame,
      CGALDecimationTechnique _DecimationTechnique) :
    m_Faces({"InitialModelNumberOfFaces", _Faces}),
    m_Sigma({"InitialModelSigma", _Sigma}),
    m_LVClosingRadius({"InitialModelLVClosingRadius", _LVClosingRadius}),
    m_GeneralClosingRadius({"InitialModelGeneralClosingRadius", _GeneralClosingRadius}),
    m_PreserveEdges({"InitialModelPreserveEdges", _PreserveEdges}),
    m_Frame({"InitialModelFrame", _Frame}),
    m_DecimationTechnique({"DecimationTechnique", static_cast<unsigned int>(_DecimationTechnique)}) {}

void
InitialModelParameters
::SerializeJSON(const std::string &fileName) {

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
InitialModelParameters
::SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) {

  dv::serialize(writer, this->m_Faces);
  dv::serialize(writer, this->m_Sigma);
  dv::serialize(writer, this->m_LVClosingRadius);
  dv::serialize(writer, this->m_GeneralClosingRadius);
  dv::serialize(writer, this->m_PreserveEdges);
  dv::serialize(writer, this->m_Frame);
  dv::serialize(writer, this->m_DecimationTechnique);

}

void
InitialModelParameters
::DeserializeJSON(const std::string &fileName)
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
InitialModelParameters
::DeserializeJSON(const rapidjson::Document& d) {

  dv::check_and_set_uint(d, this->m_Faces.second, this->m_Faces.first);
  dv::check_and_set_double(d, this->m_Sigma.second, this->m_Sigma.first);
  dv::check_and_set_uint(d, this->m_LVClosingRadius.second, this->m_LVClosingRadius.first);
  dv::check_and_set_uint(d, this->m_GeneralClosingRadius.second, this->m_GeneralClosingRadius.first);
  dv::check_and_set_bool(d, this->m_PreserveEdges.second, this->m_PreserveEdges.first);
  dv::check_and_set_uint(d, this->m_Frame.second, this->m_Frame.first);
  // TODO: Handle case where input is invalid
  dv::check_and_set_uint(d, this->m_DecimationTechnique.second, this->m_DecimationTechnique.first);

}

unsigned int
InitialModelParameters
::GetFaces() const {
  return this->m_Faces.second;
}

void
InitialModelParameters
::SetFaces(const unsigned int faces) {
  this->m_Faces.second = faces;
}

double
InitialModelParameters
::GetSigma() const {
  return this->m_Sigma.second;
}

void
InitialModelParameters
::SetSigma(const double sigma) {
  this->m_Sigma.second = sigma;
}

unsigned int
InitialModelParameters
::GetLVClosingRadius() const {
  return this->m_LVClosingRadius.second;
}

void
InitialModelParameters
::SetLVClosingRadius(const unsigned int radius) {
  this->m_LVClosingRadius.second = radius;
}

unsigned int
InitialModelParameters
::GetGeneralClosingRadius() const {
  return this->m_GeneralClosingRadius.second;
}

void
InitialModelParameters
::SetGeneralClosingRadius(const unsigned int radius) {
  this->m_GeneralClosingRadius.second = radius;
}

bool
InitialModelParameters
::GetPreserveEdges() const {
  return this->m_PreserveEdges.second;
}

void
InitialModelParameters
::SetPreserveEdges(const bool preserve) {
  this->m_PreserveEdges.second = preserve;
}

unsigned int
InitialModelParameters
::GetFrame() const {
  return this->m_Frame.second;
}

void
InitialModelParameters
::SetFrame(const unsigned int frame) {
  this->m_Frame.second = frame;
}

CGALDecimationTechnique
InitialModelParameters
::GetDecimationTechnique() const {
  return static_cast<CGALDecimationTechnique>(this->m_DecimationTechnique.second);
}

void
InitialModelParameters
::SetDecimationTechnique(const CGALDecimationTechnique technique) {
  this->m_DecimationTechnique.second = static_cast<unsigned int>(technique);
}


}

std::ostream& operator<<(std::ostream& os, const sissr::InitialModelParameters& obj) {
  os << "Faces: " << obj.GetFaces();
  os << "Sigma: " << obj.GetSigma();
  os << "LV Closing Radius: " << obj.GetLVClosingRadius();
  os << "General Closing Radius: " << obj.GetGeneralClosingRadius();
  os << "Preserve Edges: " << obj.GetPreserveEdges();
  os << "Frame: " << obj.GetFrame();
  os << "Decimation Technique: " << obj.GetDecimationTechnique();
  return os;
}
