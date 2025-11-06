#include "FoamBCBase.h"
#include "FoamPostprocessorBCBase.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"

InputParameters
FoamPostprocessorBCBase::validParams()
{
  auto params = FoamBCBase::validParams();

  params.addParam<PostprocessorName>("pp", "optional postprocessor to be used in BC");

  return params;
}

FoamPostprocessorBCBase::FoamPostprocessorBCBase(const InputParameters & params)
  : FoamBCBase(params),
    PostprocessorInterface(this),
    _pp_name(params.get<PostprocessorName>("pp")),
    _pp_value(getPostprocessorValueByName(_pp_name))
{
}

void
FoamPostprocessorBCBase::addInfoRow(BCInfoTable & table)
{
  table.addRow(name(), type(), foamVariable(), moosePostprocessor(), listFromVector(boundary()));
}
