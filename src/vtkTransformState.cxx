#include <vtkTransformState.h>

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <array>
#include <iostream>

namespace vtk
{

void
TransformState
::CaptureState(vtkSmartPointer<vtkTransform> const transform)
{
  for (unsigned int r = 0; r < 4; ++r)
    for (unsigned int c = 0; c < 4; ++c)
      this->matrix[r][c] = transform->GetMatrix()->GetElement(r,c);
}
  
void
TransformState
::RestoreState(vtkSmartPointer<vtkTransform> const transform) const
{
  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  for (unsigned int r = 0; r < 4; ++r)
    for (unsigned int c = 0; c < 4; ++c)
      mat->SetElement(r,c,this->matrix[r][c]); 

  transform->Identity();
  transform->SetMatrix( mat );
}

void
TransformState
::SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer, const std::string &key)
{
  writer.Key(key.c_str());
  writer.StartArray();
  for (unsigned int r = 0; r < 4; ++r)
    for (unsigned int c = 0; c < 4; ++c)
      writer.Double(this->matrix[r][c]);
  writer.EndArray();
}

void
TransformState
::DeserializeJSON(const rapidjson::Document &d, const std::string &key)
{
  if (d.HasMember(key.c_str()) && d[key.c_str()].IsArray() && d[key.c_str()].Size() == 16)
    {
    for (unsigned int r = 0; r < 4; ++r)
      for (unsigned int c = 0; c < 4; ++c)
        this->matrix[r][c] = d[key.c_str()][r * 4 + c].GetDouble();
    }
}

} // end namespace vtk
