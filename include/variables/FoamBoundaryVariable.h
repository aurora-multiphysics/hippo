#pragma once

#include "fvCFD_moose.h"
#include "InputParameters.h"
#include "FoamFieldBase.h"

template <typename T>
class FoamBoundaryVariable : public FoamFieldBase
{
public:
  static InputParameters validParams();

  FoamBoundaryVariable(const InputParameters & params);

  // Direct transfers write the registered fields; there is no mirror variable.
  void transferVariable() override {}
  std::string foamVariable() const override { return {}; }

  FoamBoundaryVariable(const FoamBoundaryVariable &) = delete;
  FoamBoundaryVariable & operator=(const FoamBoundaryVariable &) = delete;

  FoamBoundaryVariable(FoamBoundaryVariable &&) = delete;
  FoamBoundaryVariable & operator=(FoamBoundaryVariable &&) = delete;

private:
  std::map<BoundaryName, Foam::Field<T>> _data;
};

using FoamBoundaryScalarVariable = FoamBoundaryVariable<Foam::scalar>;
using FoamBoundaryVectorVariable = FoamBoundaryVariable<Foam::vector>;
