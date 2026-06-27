#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include <fvMesh.H>

class FoamProblem;
class FoamMesh;

class HippoInterface
{
public:
  HippoInterface(const MooseObject * moose_object);

protected:
  FoamProblem & _foam_problem;
  Foam::fvMesh & _foam_fvmesh;

private:
  FoamProblem & extractFoamProblem(const MooseObject *);
};
