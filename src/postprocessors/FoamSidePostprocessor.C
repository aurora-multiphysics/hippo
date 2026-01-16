#include "FoamSidePostprocessor.h"
#include "InputParameters.h"
#include "MooseTypes.h"

InputParameters
FoamSidePostprocessor::validParams()
{
  auto params = FoamPostprocessorBase::validParams();
  params.addRequiredParam<std::vector<SubdomainName>>(
      "boundary", "List of boundaries where postprocessor applies.");
  return params;
}

FoamSidePostprocessor::FoamSidePostprocessor(const InputParameters & params)
  : FoamPostprocessorBase(params), _boundary(params.get<std::vector<SubdomainName>>("boundary"))
{
  for (auto & boundary : _boundary)
  {
    if (_foam_mesh->boundary().findIndex(boundary) == -1)
      mooseError("Boundary '", boundary, "' not found in FoamMesh.");
  }
}
