#include "FoamFieldBase.h"
#include "FoamProblem.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "hippoUtils.h"
#include <functional>

registerMooseObject("hippoApp", FoamVariableField);

InputParameters
FoamVariableField::validParams()
{
  auto params = FoamFieldBase::validParams();
  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");
  params.addParam<bool>("create_aux_variable",
                        true,
                        "Mirror the native field into an AuxVariable. If false, register native "
                        "patch-field references for direct transfers instead.");

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
  registerTransfer();
}

void
FoamVariableField::registerTransfer()
{
  if (_create_aux_variable)
    return;

  BoundaryTransferContainer<Foam::scalar>::FieldRefMap fields;
  for (const auto subdomain : getFoamMesh().getSubdomainList())
  {
    Foam::scalarField & field =
        getFoamMesh().getBCField<Foam::volScalarField, Foam::scalar>(subdomain, _foam_variable);
    fields.emplace(getFvMesh().boundary()[subdomain].name(), std::ref(field));
  }

  // OpenFOAM owns the fields; leave registration open for additional disjoint patches.
  getFoamProblem().addBoundaryTransfer<Foam::scalar>(name(), TransferQuantity::Value, fields);
}

void
FoamVariableField::transferVariable()
{
  if (!_create_aux_variable)
    return;

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
