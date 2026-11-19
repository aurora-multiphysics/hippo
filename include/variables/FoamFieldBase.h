#pragma once

#include "HippoObject.h"

class FoamFieldBase : public HippoObject
{
public:
  static InputParameters validParams();

  /// Resolve mirroring defaults consistently for the action and variable object.
  static bool mirrorEnabled(const InputParameters & params);

  explicit FoamFieldBase(const InputParameters & params);

  // Called by FoamProblem before the base problem's initial setup and INITIAL transfers.
  virtual void initialSetup() {}

  /// Update field values and optionally mirror them into a MOOSE variable.
  virtual void syncVariable() = 0;

  // returns the name of the foam variable this object shadows
  virtual std::string foamVariable() const = 0;
};
