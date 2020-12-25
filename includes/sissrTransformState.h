#ifndef sissr_TransformState_h
#define sissr_TransformState_h

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <array>
#include <iostream>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

namespace sissr {

class TransformState
{

public:

  double matrix[4][4] = {{0,0,0,0},
                         {0,0,0,0},
                         {0,0,0,0},
                         {0,0,0,0}};

  void CaptureState(vtkSmartPointer<vtkTransform> const transform);
  void RestoreState(vtkSmartPointer<vtkTransform> const transform) const;

  void SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer, const std::string &key);
  void DeserializeJSON(const rapidjson::Document &d, const std::string &key);

}; // end class

} // namespace sissr

#endif
