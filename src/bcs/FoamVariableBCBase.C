#include "FEProblemBase.h"
#include "FoamVariableBCBase.h"

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
FoamVariableBCBase::validParams()
{
  InputParameters params = FoamBCBase::validParams();

  params.addParam<VariableName>(
      "v",
      "Optional variable to use in BC. This allows existing AuxVariables to be"
      " used rather than creating a new one under the hood.");
  // Get desired parameters from Variable objects
  params.transferParam<std::vector<Real>>(MooseVariable::validParams(), "initial_condition");

  return params;
}

FoamVariableBCBase::FoamVariableBCBase(const InputParameters & params)
  : FoamBCBase(params), _moose_var(nullptr)
{
}

void
FoamVariableBCBase::initialSetup()
{
  // Check variable exists
  auto var_name = parameters().isParamValid("v") ? parameters().get<VariableName>("v") : _name;
  if (!_c_fe_problem.hasVariable(var_name))
    mooseError("Variable '", var_name, "' doesn't exist");

  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  _moose_var = &_c_fe_problem.getVariable(tid, var_name);

  // Check variable is constant monomial in case it is provided.
  if (!is_constant_monomial(*_moose_var))
    mooseError("Variable '", var_name, "' must be a constant monomial.");
}

void
FoamVariableBCBase::addInfoRow(BCInfoTable & table)
{
  // List info about BC
  table.addRow(name(), type(), foamVariable(), mooseVariable(), listFromVector(boundary()));
}

Real
FoamVariableBCBase::variableValueAtElement(const libMesh::Elem * elem)
{
  auto & sys = _moose_var->sys();
  auto dof = elem->dof_number(sys.number(), _moose_var->number(), 0);
  return sys.solution()(dof);
}

std::vector<Real>
FoamVariableBCBase::getMooseVariableArray(int subdomainId)
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
