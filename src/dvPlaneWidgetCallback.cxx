#include <dvSiSSRView.h>
#include <dvPlaneWidgetCallback.h>

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
