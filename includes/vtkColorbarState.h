#ifndef vtk_ColorbarState_h
#define vtk_ColorbarState_h

#include <string>

namespace sissr {

struct ColorbarState
{
  std::string Title;
  double Min;
  double Max;
};

} // namespace sissr

#endif
