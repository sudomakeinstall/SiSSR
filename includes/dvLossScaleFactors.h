#ifndef dv_LossScaleFactors_h
#define dv_LossScaleFactors_h

namespace dv
{

struct LossScaleFactors
{
  LossScaleFactors(const double _Robust,
                   const double _Velocity,
                   const double _Acceleration,
                   const double _ThinPlate,
                   const double _TriangleAspectRatio,
                   const double _EdgeLength) :
    Robust(_Robust),
    Velocity(_Velocity),
    Acceleration(_Acceleration),
    ThinPlate(_ThinPlate),
    TriangleAspectRatio(_TriangleAspectRatio),
    EdgeLength(_EdgeLength) {};
  LossScaleFactors() :
    Robust(1.0),
    Velocity(0.0),
    Acceleration(0.0),
    ThinPlate(0.0),
    TriangleAspectRatio(0.0),
    EdgeLength(0.0) {};
  double Robust;
  double Velocity;
  double Acceleration;
  double ThinPlate;
  double TriangleAspectRatio;
  double EdgeLength;
};

}

#endif
