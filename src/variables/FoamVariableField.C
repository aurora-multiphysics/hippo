#include "FoamVariableBase.h"
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "Registry.h"

registerMooseObject("hippoApp", FoamVariableField);

FoamVariableField::FoamVariableField(const InputParameters & params) : FoamVariableBase(params) {}
