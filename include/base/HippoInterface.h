#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include <fvMesh.H>
#include <string>

class FoamProblem;
class FoamMesh;

class HippoInterface
{
public:
  HippoInterface(const MooseObject * moose_object);

protected:
  FoamProblem & _foam_problem;
  FoamMesh & _mesh;
  Foam::fvMesh & _fv_mesh;
  Foam::Time & _foam_time;

private:
  FoamProblem & extractFoamProblem(const MooseObject *);
};
