#include <sissrInitialModelParameters.h>

#include <dvRapidJSONHelper.h>

namespace sissr {

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

}
