#ifndef sissr_InitialModelParameters_h
#define sissr_InitialModelParameters_h

namespace sissr {

struct InitialModelParameters
{

  InitialModelParameters(
      unsigned int        _NumberOfFaces,
      double              _Sigma,
      unsigned int        _LVClosingRadius,
      unsigned int        _GeneralClosingRadius,
      bool                _PreserveEdges,
      unsigned int        _Frame,
      CGALDecimationTechnique _DecimationTechnique) :
    NumberOfFaces(_NumberOfFaces),
    Sigma(_Sigma),
    LVClosingRadius(_LVClosingRadius),
    GeneralClosingRadius(_GeneralClosingRadius),
    PreserveEdges(_PreserveEdges),
    Frame(_Frame),
    DecimationTechnique(_DecimationTechnique) {}

  unsigned int            NumberOfFaces;
  double                  Sigma;
  unsigned int            LVClosingRadius;
  unsigned int            GeneralClosingRadius;
  bool                    PreserveEdges;
  unsigned int            Frame;
  CGALDecimationTechnique DecimationTechnique;

  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
  void DeserializeJSON(const rapidjson::Document& d);

};

} // namespace sissr

#endif
