#pragma once

#include "FoamPostprocessorBase.h"
#include "MooseTypes.h"

class FoamSidePostprocessor : public FoamPostprocessorBase
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);

protected:
  std::vector<SubdomainName> _boundary;
};
