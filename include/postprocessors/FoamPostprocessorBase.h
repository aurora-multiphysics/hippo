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

  virtual void initialize() final;

  virtual void execute() final;

  virtual void finalize() final;

  virtual void threadJoin(const UserObject & uo) final;

  virtual void compute() = 0;

protected:
  Foam::fvMesh * _foam_mesh;
};
