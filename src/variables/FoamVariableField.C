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

  return params;
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : FoamFieldBase(params), _foam_variable(params.get<std::string>("foam_variable"))
{
}

void
FoamVariableField::transferVariable()
{
  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  auto & moose_var = getFoamProblem().getVariable(tid, _name);

  // Loop through subdomains extracting foam_variable and setting on libMesh elements
  for (auto subdomain : getFoamMesh().getSubdomainList())
  {
    auto & var = getFvMesh().boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _foam_variable);
    Hippo::internal::copyFieldFoamToMoose(getFoamMesh(), var, moose_var, subdomain);
  }

  moose_var.sys().solution().close();
}
