#pragma once

#include "InputParameters.h"
#include "MooseObject.h"

class FoamProblem;

class HippoBase : public MooseObject
{
public:
  static InputParameters validParams();
  HippoBase(const InputParameters & params);

  FoamProblem & getFoamProblem() const;
};
