#pragma once

#include "HippoObject.h"

class FoamFieldBase : public HippoObject
{
public:
  static InputParameters validParams();

  explicit FoamFieldBase(const InputParameters & params);

  // Called by FoamProblem before the base problem's initial setup and INITIAL transfers.
  virtual void initialSetup() {}

  // transfer variable from OpenFOAM field to MOOSE variable
  virtual void transferVariable() = 0;

  // returns the name of the foam variable this object shadows
  virtual std::string foamVariable() const = 0;
};
