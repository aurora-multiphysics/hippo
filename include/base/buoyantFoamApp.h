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
  /**
   * Append the patch's face temperatures to the vector T.
   *
   * Return the number of items added.
   */
  size_t append_patch_face_T(int patch_id, std::vector<double> & T);

  size_t patch_size(int patch_id);

  /**
   * Apply the temperatures as a fixedValue boundary condition on the patch.
   *
   * @param patch_id
   * @param moose_T
   */
  void set_patch_face_t(int patch_id, const std::vector<double> & moose_T);

  /**
   * Apply the negative heat flux as a fixedGradient boundary condition to the given patch.
   *
   * Note that typically this heat flux will be coming from a boundary that interfaces the given
   * OpenFOAM boundary; this heat flux will hence be in the opposite direction in OpenFOAM's
   * coordinate system, meaning we should take the negative. By Fourier's law (q = -k.∇T), to
   * convert heat flux to temperature gradient we need to take the negative of the heat flux again.
   * Therefore it is convenient to provide an interface to set the *negative* heat flux, to avoid
   * applying a minus sign twice.
   */
  void set_patch_face_negative_heat_flux(int patch_id, const std::vector<double> & negative_hf);

  std::unique_ptr<buoyantFoamImpl> _impl;
};
}
