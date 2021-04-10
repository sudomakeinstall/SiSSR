#ifndef sissr_CGALDecimationTechnique_h
#define sissr_CGALDecimationTechnique_h

// STD
#include <iostream>

namespace sissr {

  enum class CGALDecimationTechnique {
    Midpoint,
    LindstromTurk
  };

}

std::ostream& operator<<(std::ostream& os, const sissr::CGALDecimationTechnique& obj);

#endif
