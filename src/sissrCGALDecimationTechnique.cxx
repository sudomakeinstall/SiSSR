// SiSSR
#include <sissrCGALDecimationTechnique.h>

std::ostream& operator<<(std::ostream& os, const sissr::CGALDecimationTechnique& obj) {
  switch (obj) {
    case sissr::CGALDecimationTechnique::Midpoint:
      os << "Midpoint";
      break;
    case sissr::CGALDecimationTechnique::LindstromTurk:
      os << "LindstromTurk";
      break;
  }
  return os;
}
