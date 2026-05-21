#pragma once

#include "HippoBase.h"

class FoamFieldBase : public MooseObject, public HippoBase
{
public:
  static InputParameters validParams();

  explicit FoamFieldBase(const InputParameters & params);

  // transfer variable from OpenFOAM field to MOOSE variable
  virtual void transferVariable() = 0;

  // returns the name of the foam variable this object shadows
  virtual std::string foamVariable() const = 0;
};
