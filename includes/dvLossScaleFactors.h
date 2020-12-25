#ifndef dv_LossScaleFactors_h
#define dv_LossScaleFactors_h

namespace sissr {

struct LossScaleFactors
{
  // Use defaults specified in the paper:
  // SiSSR: Simultaneous subdivision surface registration for the
  // quantification of cardiac function from computed tomography in canines.
  // https://www.ncbi.nlm.nih.gov/pubmed/29627686
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
    Acceleration(0.1),
    ThinPlate(0.1),
    TriangleAspectRatio(10.0),
    EdgeLength(0.0) {};
  double Robust;
  double Velocity;
  double Acceleration;
  double ThinPlate;
  double TriangleAspectRatio;
  double EdgeLength;
};

} // namespace sissr

#endif
