#ifndef vtk_ColorbarState_h
#define vtk_ColorbarState_h

#include <string>

namespace vtk
{

struct ColorbarState
{
  std::string Title;
  double Min;
  double Max;
};


}

#endif
