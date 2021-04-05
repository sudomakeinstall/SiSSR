#ifndef sissr_InitialModelParameters_h
#define sissr_InitialModelParameters_h

// SiSSR
#include <sissrCGALDecimationTechnique.h>

// RapidJSON
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace sissr {

struct InitialModelParameters
{

  InitialModelParameters(
      unsigned int        _Faces,
      double              _Sigma,
      unsigned int        _LVClosingRadius,
      unsigned int        _GeneralClosingRadius,
      bool                _PreserveEdges,
      unsigned int        _Frame,
      CGALDecimationTechnique _DecimationTechnique) :
    m_Faces({"Faces", _Faces}),
    m_Sigma({"Sigma", _Sigma}),
    m_LVClosingRadius({"LVClosingRadius", _LVClosingRadius}),
    m_GeneralClosingRadius({"GeneralClosingRadius", _GeneralClosingRadius}),
    m_PreserveEdges({"PreserveEdges", _PreserveEdges}),
    m_Frame({"Frame", _Frame}),
    m_DecimationTechnique({"DecimationTechnique", static_cast<unsigned int>(_DecimationTechnique)}) {}

  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
  void DeserializeJSON(const rapidjson::Document& d);

  unsigned int GetFaces() const { return this->m_Faces.second; }
  void SetFaces(const unsigned int faces) { this->m_Faces.second = faces; }

  double GetSigma() const { return this->m_Sigma.second; }
  void SetSigma(const double sigma) { this->m_Sigma.second = sigma; }

  unsigned int GetLVClosingRadius() const { return this->m_LVClosingRadius.second; }
  void SetLVClosingRadius(const unsigned int radius) { this->m_LVClosingRadius.second = radius; }

  unsigned int GetGeneralClosingRadius() const { return this->m_GeneralClosingRadius.second; }
  void SetGeneralClosingRadius(const unsigned int radius) { this->m_GeneralClosingRadius.second = radius; }

  bool GetPreserveEdges() const { return this->m_PreserveEdges.second; }
  void SetPreserveEdges(const bool preserve) { this->m_PreserveEdges.second = preserve; }

  unsigned int GetFrame() const { return this->m_Frame.second; }
  void SetFrame(const unsigned int frame) { this->m_Frame.second = frame; }

  CGALDecimationTechnique GetDecimationTechnique() const {
    return static_cast<CGALDecimationTechnique>(this->m_DecimationTechnique.second);
  }
  void SetDecimationTechnique(const CGALDecimationTechnique technique) {
    this->m_DecimationTechnique.second = static_cast<unsigned int>(technique);
  }

  private:

  std::pair<const std::string, unsigned int> m_Faces;
  std::pair<const std::string, double> m_Sigma;
  std::pair<const std::string, unsigned int> m_LVClosingRadius;
  std::pair<const std::string, unsigned int> m_GeneralClosingRadius;
  std::pair<const std::string, bool> m_PreserveEdges;
  std::pair<const std::string, unsigned int> m_Frame;
  std::pair<const std::string, unsigned int> m_DecimationTechnique;

};

} // namespace sissr

#endif
