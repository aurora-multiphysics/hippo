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
  // append patches nodal (interopolated from the faces by OpenFAOM)
  // temperature to the vector T (clear it first)
  // returns number of things added
  size_t append_patchT(int patch_id, std::vector<double> & T);
  size_t append_patch_face_T(int patch_id, std::vector<double> & T);
  // TODO: Delete me
  size_t append_faceT(int patch_id, std::vector<double> & T);
  std::unique_ptr<buoyantFoamImpl> _impl;
};
}
