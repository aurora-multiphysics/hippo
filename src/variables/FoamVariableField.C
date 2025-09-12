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

  auto & mesh = problem->mesh().fvMesh();
  _field_shadow = &mesh.lookupObject<Foam::volScalarField>(_foam_variable);
}

void
FoamVariableField::transferVariable()
{
}
