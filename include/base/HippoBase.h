#pragma once

#include "InputParameters.h"
#include "MooseObject.h"

class FoamProblem;

class HippoBase
{
public:
  HippoBase(const MooseObject * moose_object);

  FoamProblem & getFoamProblem() const;

private:
  FoamProblem * extractFoamProblemPtr(const MooseObject *);
  FoamProblem * const _foam_problem;
};
