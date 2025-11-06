#pragma once

#include "FoamBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

class FoamPostprocessorBCBase : public FoamBCBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  explicit FoamPostprocessorBCBase(const InputParameters & params);

  // returns the moose AuxVariable imposed on OpenFOAM
  VariableName moosePostprocessor() const { return _pp_name; }

  virtual void initialSetup() {};

  virtual void addInfoRow(BCInfoTable & table);

protected:
  const PostprocessorName _pp_name;

  // Pointer to Moose variable used to impose BC
  const PostprocessorValue & _pp_value;
};
