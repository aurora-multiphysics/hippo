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
  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");

  // Get desired parameters from Variable objects
  params.transferParam<std::vector<Real>>(MooseVariable::validParams(), "initial_condition");

  params.registerBase("FoamVariable");
  params.registerSystemAttributeName("FoamVariable");
  return params;
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : MooseObject(params), _foam_variable(params.get<std::string>("foam_variable"))
{
  // Already checked in createMooseVariable
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());

  _mesh = &problem->mesh();
}

void
FoamVariableField::transferVariable()
{
  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  auto & moose_var = getMooseApp().feProblem().getVariable(tid, _name);

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
      auto dof_t = elem_ptr->dof_number(moose_var.sys().number(), moose_var.number(), 0);
      moose_var.sys().solution().set(dof_t, var[j]);
    }
  }

  moose_var.sys().solution().close();
}
