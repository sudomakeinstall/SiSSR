#ifndef dv_PlaneWidgetCallback_h
#define dv_PlaneWidgetCallback_h

#include <vtkPlaneWidget.h>
#include <vtkPlaneSource.h>
#include <vtkCommand.h>

namespace sissr {

class SiSSRView;

/*
 A callback function which keeps the position of a vtkPlaneSource
 in sync with a vtkPlaneWidget.
 */
class PlaneWidgetCallback: public vtkCommand
{
public:
static PlaneWidgetCallback *New();

void Execute(vtkObject*, unsigned long, void*) override;

SiSSRView* window;
vtkPlaneWidget* widget;
vtkPlaneSource* source;

};

} // namespace sissr

#endif
