#include "FoamProblem.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "MooseVariableConstMonomial.h"
#include "Registry.h"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamVariableField);

InputParameters
FoamVariableField::validParams()
{
  InputParameters params = MooseVariableConstMonomial::validParams();
  params.addRequiredParam<std::string>("foam_variable", "Foam variable to be shadowed");
  return params;
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : MooseVariableConstMonomial(params), _foam_variable(params.get<std::string>("foam_variable"))
{
  if (kind() != Moose::VarKindType::VAR_AUXILIARY)
    mooseError("FoamVariables can only be instantiated as AuxVariables");

  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());
  if (!problem)
    mooseError("This Variable can only be used with FoamProblem");

  _mesh = &problem->mesh();
  _field_shadow = &_mesh->fvMesh().lookupObject<Foam::volScalarField>(_foam_variable);

  problem->addShadowVariable(this);
}

void
FoamVariableField::transferVariable()
{
  auto subdomains = _mesh->getSubdomainList();
  auto & foam_mesh = _mesh->fvMesh();
  std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
  std::vector<Foam::scalar> data;

  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    auto patch_id = subdomains[i];
    auto & var = foam_mesh.boundary()[patch_id].lookupPatchField<Foam::volScalarField, double>(
        _foam_variable);
    std::copy(var.begin(), var.end(), std::back_inserter(data));
    patch_counts[i] = var.size();
  }

  std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    for (auto elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
    {
      auto elem_ptr = _mesh->getElemPtr(elem + _mesh->rank_element_offset);
      assert(elem_ptr);
      auto dof_t = elem_ptr->dof_number(sys().number(), number(), 0);
      sys().solution().set(dof_t, data[elem]);
    }
  }

  sys().solution().close();
}
