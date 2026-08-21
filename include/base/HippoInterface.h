#pragma once

#include "InputParameters.h"
#include "MooseObject.h"
#include <fvMesh.H>

class FoamProblem;
class FoamMesh;

/*
HippoInterface is a base class containing convenient access to common
Hippo and Foam objects such as Foam::Time, Foam::fvMesh and
FoamProblem.
*/
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
