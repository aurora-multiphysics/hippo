#pragma once

#include "InputParameters.h"
#include "Postprocessor.h"
#include "ElementUserObject.h"
#include "fvMesh.H"

class FoamPostprocessorBase : public ElementUserObject, public Postprocessor
{
public:
  static InputParameters validParams();

  FoamPostprocessorBase(const InputParameters & params);

  // We dont want the usual UserObject functions to be executed
  // But we still want the Foam Postprocessors to be reported with the other
  // Foam postprocessors
  virtual void initialize() final;

  virtual void execute() final;

  virtual void finalize() final;

  virtual void threadJoin(const UserObject & uo) final;

  // Compute postprocessor, to be called within FoamProblem
  virtual void compute() = 0;

protected:
  Foam::fvMesh * _foam_mesh;
};
