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
      CGALDecimationTechnique _DecimationTechnique);

  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
  void DeserializeJSON(const rapidjson::Document& d);

  unsigned int GetFaces() const;
  void SetFaces(const unsigned int faces);

  double GetSigma() const;
  void SetSigma(const double sigma);

  unsigned int GetLVClosingRadius() const;
  void SetLVClosingRadius(const unsigned int radius);

  unsigned int GetGeneralClosingRadius() const;
  void SetGeneralClosingRadius(const unsigned int radius);

  bool GetPreserveEdges() const;
  void SetPreserveEdges(const bool preserve);

  unsigned int GetFrame() const;
  void SetFrame(const unsigned int frame);

  CGALDecimationTechnique GetDecimationTechnique() const;
  void SetDecimationTechnique(const CGALDecimationTechnique technique);

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
