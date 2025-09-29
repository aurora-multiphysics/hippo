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
  auto params = MooseObject::validParams();

  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");

  // For handling the deprecated variable shadowing syntax
  params.addPrivateParam<bool>("_deprecated", false);
  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

MooseVariableFieldBase &
FoamVariableField::createMooseVariable(std::string name, const InputParameters & params)
{
  // TODO: Add other parameters from variable instantiations such as Boundary restrictions
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());
  if (!problem)
    mooseError("This Variable can only be used with FoamProblem");

  auto var_params = _factory.getValidParams("MooseVariable");

  // The MOOSE variable has to be constant monomial
  var_params.set<MooseEnum>("order") = "CONSTANT";
  var_params.set<MooseEnum>("family") = "MONOMIAL";

  // In the deprecated variable shadowing system, the variable already exists
  if (params.get<bool>("_deprecated"))
    return problem->getVariable(0, name);

  // Create the Aux variable
  problem->addAuxVariable("MooseVariable", name, var_params);

  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");

  // return reference to moose variable
  return problem->getVariable(tid, name, Moose::VarKindType::VAR_AUXILIARY);
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : MooseObject(params),
    _foam_variable(params.get<std::string>("foam_variable")),
    _moose_var(createMooseVariable(name(), params))
{
  // Already checked in createMooseVariable
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());

  _mesh = &problem->mesh();
}

void
FoamVariableField::transferVariable()
{
  // TODO: some recent  changes to FoamMesh can improve this
  auto & foam_mesh = _mesh->fvMesh();
  for (auto subdomain : _mesh->getSubdomainList())
  {
    size_t patch_count = _mesh->getPatchCount(subdomain);
    size_t patch_offset = _mesh->getPatchOffset(subdomain);

    auto & var = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _foam_variable);
    for (size_t j = 0; j < patch_count; ++j)
    {
      auto elem = patch_offset + j;
      auto elem_ptr = _mesh->getElemPtr(elem + _mesh->rank_element_offset);
      assert(elem_ptr);
      auto dof_t = elem_ptr->dof_number(_moose_var.sys().number(), _moose_var.number(), 0);
      _moose_var.sys().solution().set(dof_t, var[j]);
    }
  }

  _moose_var.sys().solution().close();
}
