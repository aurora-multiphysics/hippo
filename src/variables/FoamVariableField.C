#include "FoamProblem.h"
#include "FoamVariableBase.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "MooseVariableConstMonomial.h"
#include "Registry.h"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamVariableField);

FoamVariableField::FoamVariableField(const InputParameters & params) : FoamVariableBase(params)
{
  _field_shadow = &_mesh->fvMesh().lookupObject<Foam::volScalarField>(_foam_variable);
}
