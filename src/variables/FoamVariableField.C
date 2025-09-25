#include "FoamVariableField.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "MooseVariableFieldBase.h"

registerMooseObject("hippoApp", FoamVariableField);

InputParameters
FoamVariableField::validParams()
{
  auto params = MooseVariableFieldBase::validParams();
  params.remove("family");
  params.remove("type");

  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");

  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

MooseVariableFieldBase &
FoamVariableField::getVariable(std::string name, const InputParameters & params)
{
  auto & problem = getMooseApp().feProblem();

  InputParameters var_params(params);
  auto valid_params = _factory.getValidParams("MooseVariable");

  for (auto & param : params)
    if (!valid_params.isParamValid(param.first))
      var_params.remove(param.first);

  var_params.set<std::string>("order") = "CONSTANT";
  var_params.set<std::string>("family") = "MONOMIAL";

  problem.addAuxVariable("MooseVariable", _name, var_params);

  return problem.getVariable(0, _name, Moose::VarKindType::VAR_AUXILIARY);
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : MooseObject(params),
    _foam_variable(params.get<std::string>("foam_variable")),
    _moose_var(getVariable(_name, params))
{
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());
  if (!problem)
    mooseError("This Variable can only be used with FoamProblem");

  _mesh = &problem->mesh();
}

void
FoamVariableField::transferVariable()
{
  // TODO: some recent  changes to FoamMesh can improve this
  // TODO: may be useful to create a base class for getting data to and from MOOSE/OpenFOAM

  auto subdomains = _mesh->getSubdomainList();
  auto & foam_mesh = _mesh->fvMesh();
  std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
  std::vector<Foam::scalar> data;

  // loop over subdomains and construct vector with boundary values
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    auto patch_id = subdomains[i];
    auto & var = foam_mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>(
        _foam_variable);
    std::copy(var.begin(), var.end(), std::back_inserter(data));
    patch_counts[i] = var.size();
  }

  // assign values to each element
  std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    for (auto elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
    {
      auto elem_ptr = _mesh->getElemPtr(elem + _mesh->rank_element_offset);
      assert(elem_ptr);
      auto dof_t = elem_ptr->dof_number(_moose_var.sys().number(), _moose_var.number(), 0);
      _moose_var.sys().solution().set(dof_t, data[elem]);
    }
  }

  _moose_var.sys().solution().close();
}
