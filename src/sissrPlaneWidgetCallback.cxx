// SiSSR
#include <sissrView.h>
#include <sissrPlaneWidgetCallback.h>

namespace sissr {

PlaneWidgetCallback*
PlaneWidgetCallback
::New()
{
  return new PlaneWidgetCallback;
}

void
PlaneWidgetCallback
::Execute(vtkObject*, unsigned long, void*)
{
  this->window->PlaneWidgetDidMove();
}

} // namespace sissr
