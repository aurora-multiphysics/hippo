#pragma once

#include "FoamInterface.h"

namespace Hippo
{
class buoyantFoamImpl;
class buoyantFoamApp
{
  FoamInterface * _interface;

public:
  ~buoyantFoamApp();
  buoyantFoamApp(FoamInterface * _interface);
  bool run();
  // Append the patch's face temperatures to the vector T.
  // Return the number of items added.
  size_t append_patch_face_T(int patch_id, std::vector<double> & T);
  size_t patch_size(int patch_id);
  void set_patch_face_t(int patch_id, const std::vector<double> & moose_T);
  std::unique_ptr<buoyantFoamImpl> _impl;
};
}
