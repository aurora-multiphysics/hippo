#include "Coupleable.h"
#include "FoamBCBase.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "Registry.h"
#include "volFieldsFwd.H"
#include <algorithm>
#include <vector>

// remove after improved tests
// registerMooseObject("hippoApp", FoamBCBase);

namespace
{
// Private function to check if variables are constant monomials
inline bool
is_constant_monomial(const MooseVariableFieldBase & var)
{
  return var.order() == libMesh::Order::CONSTANT && var.feType().family == FEFamily::MONOMIAL;
}
}

InputParameters
FoamBCBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "Name of a Foam field. e.g. T (temperature) U (velocity).");
  params.addRequiredCoupledVar("v", "MOOSE variable to impose as the boundary condition.");
  params.addParam<std::vector<SubdomainName>>("boundary",
                                              "Boundaries that the boundary condition applies to.");

  params.registerSystemAttributeName("FoamBC");
  params.registerBase("FoamBC");

  return params;
}

FoamBCBase::FoamBCBase(const InputParameters & params)
  : MooseObject(params),
    Coupleable(this, false),
    _foam_variable(params.get<std::string>("foam_variable")),
    _v(getVariable(params)),
    _boundary(params.get<std::vector<SubdomainName>>("boundary"))
{
  auto * problem = dynamic_cast<FoamProblem *>(&_c_fe_problem);
  if (!problem)
    mooseError("FoamBC system can only be used with FoamProblem");

  _mesh = &problem->mesh();

  // check that the foam variable exists
  if (!_mesh->fvMesh().foundObject<Foam::volScalarField>(_foam_variable))
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

const MooseVariableFieldBase &
FoamBCBase::getVariable(const InputParameters & params)
{
  auto variable_name = *params.getCoupledVariableParamNames().begin();
  auto * var = getFieldVar(variable_name, 0);
  if (!is_constant_monomial(*var))
  {
    mooseError("Variable '", variable_name, "' (", var->name(), ") must be a constant monomial");
  }
  return *var;
}

Real
FoamBCBase::variableValueAtElement(const libMesh::Elem * elem)
{
  auto & sys = _v.sys();
  auto dof = elem->dof_number(sys.number(), _v.number(), 0);
  return sys.solution()(dof);
}

std::vector<Real>
FoamBCBase::getMooseVariableArray(int subdomainId)
{
  size_t patch_count = _mesh->getPatchCount(subdomainId);
  size_t patch_offset = _mesh->getPatchOffset(subdomainId);

  std::vector<Real> var_array(patch_count);
  for (size_t j = 0; j < patch_count; ++j)
  {
    auto elem = patch_offset + j;
    auto elem_ptr = _mesh->getElemPtr(elem + _mesh->rank_element_offset);
    assert(elem_ptr);
    var_array[j] = variableValueAtElement(elem_ptr);
  }

  return var_array;
}
