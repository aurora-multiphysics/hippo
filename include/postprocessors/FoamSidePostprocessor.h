#pragma once

#include "FoamPostprocessorBase.h"
#include "MooseTypes.h"

class FoamSidePostprocessor : public FoamPostprocessorBase
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);

  const std::vector<SubdomainName> & boundary() const { return _boundary; }

protected:
  std::vector<SubdomainName> _boundary;

  Real getArea();
};
