
#include "FoamBCBase.h"
#include "FoamProblem.h"

#include <Coupleable.h>
#include <InputParameters.h>
#include <MooseError.h>
#include <MooseObject.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <Registry.h>
#include <volFieldsFwd.H>

#include <algorithm>
#include <vector>

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");
  params.addParam<std::vector<SubdomainName>>("boundary",
                                              "Boundaries that the boundary condition applies to.");
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");

  params.addPrivateParam("_foam_var_settable", true);
  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params)
  : MooseObject(params),
    Coupleable(this, false),
    _foam_variable(params.get<std::string>("foam_variable")),
    _boundary(params.get<std::vector<SubdomainName>>("boundary"))
{
  auto * problem = dynamic_cast<FoamProblem *>(&_c_fe_problem);
  if (!problem)
    mooseError("FoamBC system can only be used with FoamProblem");

  _mesh = &problem->mesh();

  // check that the foam variable exists
  if (!_mesh->foamHasObject<Foam::volScalarField>(_foam_variable))
    mooseError("There is no OpenFOAM field named '", _foam_variable, "'");

  // check that the boundary is in the FoamMesh
  auto all_subdomain_names = _mesh->getSubdomainNames(_mesh->getSubdomainList());
  for (auto subdomain : _boundary)
  {
    auto it = std::find(all_subdomain_names.begin(), all_subdomain_names.end(), subdomain);
    if (it == all_subdomain_names.end())
      mooseError("Boundary '", subdomain, "' not found in FoamMesh");
  }

  if (_boundary.empty())
    _boundary = all_subdomain_names;
}
