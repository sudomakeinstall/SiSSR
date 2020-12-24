
#include <dvSiSSRView.h>
#include <dvPlaneWidgetCallback.h>

namespace dv
{

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

}
