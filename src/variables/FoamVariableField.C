#include "FoamFieldBase.h"
#include "FoamProblem.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "hippoUtils.h"

registerMooseObject("hippoApp", FoamVariableField);

InputParameters
FoamVariableField::validParams()
{
  auto params = FoamFieldBase::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");
  params.addParam<bool>("create_aux_variable",
                        true,
                        "Mirror the native field into an AuxVariable. Native field access is "
                        "registered regardless of this option.");

  return params;
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : FoamFieldBase(params),
    _foam_variable(params.get<std::string>("foam_variable")),
    _create_aux_variable(getParam<bool>("create_aux_variable"))
{
}

void
FoamVariableField::initialSetup()
{
  getFoamProblem().registerVolFieldVariable<Foam::scalar>(name(), _foam_variable);
}

void
FoamVariableField::transferVariable()
{
  if (!_create_aux_variable)
    return;

  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  auto & moose_var = getFoamProblem().getVariable(tid, _name);
  auto & accessor = getFoamProblem().getFieldAccessor<Foam::scalar>(name());

  // Copy registered boundary values onto the corresponding mirrored libMesh elements.
  for (const auto subdomain : getFoamMesh().getSubdomainList())
  {
    const auto & name = getFoamMesh().getSubdomainName(subdomain);
    const auto & field = accessor.boundaryField(name);
    Hippo::internal::copyFieldFoamToMoose(getFoamMesh(), field, moose_var, subdomain);
  }

  moose_var.sys().solution().close();
}
