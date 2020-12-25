#ifndef sissr_PlaneWidgetCallback_h
#define sissr_PlaneWidgetCallback_h

// VTK
#include <vtkPlaneWidget.h>
#include <vtkPlaneSource.h>
#include <vtkCommand.h>

namespace sissr {

class View;

/*
 A callback function which keeps the position of a vtkPlaneSource
 in sync with a vtkPlaneWidget.
 */
class PlaneWidgetCallback: public vtkCommand
{
public:
static PlaneWidgetCallback *New();

void Execute(vtkObject*, unsigned long, void*) override;

View* window;
vtkPlaneWidget* widget;
vtkPlaneSource* source;

};

} // namespace sissr

#endif
