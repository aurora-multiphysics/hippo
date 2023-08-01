#if 0
#pragma once

#include <memory>

namespace Hippo {
  namespace icoFoam {
    struct FoamAppImpl;
    class FoamApp
    {
      std::unique_ptr<FoamAppImpl> impl_;
    public:
      FoamApp(Hippo::Env &env);
      ~FoamApp();
      bool run(Hippo::Env &env);
    };
  }
}

#endif
// Local Variables:
// mode: c++
// End:
