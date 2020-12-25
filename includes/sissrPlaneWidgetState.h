#ifndef sissr_PlaneWidgetState_h
#define sissr_PlaneWidgetState_h

namespace sissr {

template<typename PlaneWidget>
class PlaneWidgetState
{

public:

  double Origin[3];
  double Point1[3];
  double Point2[3];

  void CaptureState(PlaneWidget* plane)
  {
    plane->GetOrigin(this->Origin);
    plane->GetPoint1(this->Point1);
    plane->GetPoint2(this->Point2);
  }
  void RestoreState(PlaneWidget* plane)
  {
    plane->SetOrigin(this->Origin[0],
                     this->Origin[1],
                     this->Origin[2]);
    plane->SetPoint1(this->Point1[0],
                     this->Point1[1],
                     this->Point1[2]);
    plane->SetPoint2(this->Point2[0],
                     this->Point2[1],
                     this->Point2[2]);
  }

}; // end class

} // namespace sissr

#endif
