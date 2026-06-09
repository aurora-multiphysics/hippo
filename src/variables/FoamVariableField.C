#include "FoamFieldBase.h"
#include "FoamVariableField.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "hippoUtils.h"

registerMooseObject("hippoApp", FoamVariableField);

InputParameters
FoamVariableField::validParams()
{
  auto params = FoamFieldBase::validParams();

  params.addRequiredParam<std::string>("foam_variable",
                                       "OpenFOAM variable or functionObject to be shadowed");

  // Get desired parameters from Variable objects
  params.transferParam<std::vector<Real>>(MooseVariable::validParams(), "initial_condition");

  return params;
}

FoamVariableField::FoamVariableField(const InputParameters & params)
  : FoamFieldBase(params), _foam_variable(params.get<std::string>("foam_variable"))
{
  auto * problem = dynamic_cast<FoamProblem *>(&getMooseApp().feProblem());
  if (!problem)
    mooseError("This Variable can only be used with FoamProblem");

  _mesh = &problem->mesh();
}

void
FoamVariableField::transferVariable()
{
  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  auto & moose_var = getMooseApp().feProblem().getVariable(tid, _name);

  // Loop through subdomains extracting foam_variable and setting on libMesh elements
  auto & foam_mesh = _mesh->fvMesh();
  for (auto subdomain : _mesh->getSubdomainList())
  {
    auto & var = foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(
        _foam_variable);
    Hippo::internal::copyFieldFoamToMoose(*_mesh, var, moose_var, subdomain);
  }

  moose_var.sys().solution().close();
}
