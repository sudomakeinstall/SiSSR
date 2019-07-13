#ifndef dv_PlaneWidgetCallback_h
#define dv_PlaneWidgetCallback_h

#include <vtkCommand.h>

namespace dv
{

class SubdivisionRegistrationWindow;

/*
 A callback function which keeps the position of a vtkPlaneSource
 in sync with a vtkPlaneWidget.
 */
class PlaneWidgetCallback: public vtkCommand
{
public:
static PlaneWidgetCallback *New();

void Execute(vtkObject*, unsigned long, void*) override;

SubdivisionRegistrationWindow* window;
vtkPlaneWidget* widget;
vtkPlaneSource* source;

};

}

#endif
