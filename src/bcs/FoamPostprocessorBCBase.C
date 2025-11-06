#include "FoamBCBase.h"
#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"
#include "Receiver.h"

InputParameters
FoamPostprocessorBCBase::validParams()
{
  auto params = FoamBCBase::validParams();

  params.addParam<PostprocessorName>("pp", "optional postprocessor to be used in BC");
  params.transferParam<Real>(Receiver::validParams(), "default");

  return params;
}

FoamPostprocessorBCBase::FoamPostprocessorBCBase(const InputParameters & params)
  : FoamBCBase(params),
    PostprocessorInterface(this),
    _pp_name((params.isParamSetByUser("pp")) ? params.get<PostprocessorName>("pp") : _name),
    _pp_value(getPostprocessorValueByName(_pp_name))
{
  if (params.isParamSetByUser("pp") && params.isParamSetByUser("default"))
    mooseWarning("'pp' and 'default' should not be set. 'default' ignored.");
}

void
FoamPostprocessorBCBase::addInfoRow(BCInfoTable & table)
{
  table.addRow(name(), type(), foamVariable(), moosePostprocessor(), listFromVector(boundary()));
}
