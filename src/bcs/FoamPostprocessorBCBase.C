#include "FoamBCBase.h"
#include "FoamPostprocessorBCBase.h"
#include "hippoUtils.h"

#include "InputParameters.h"
#include "MooseTypes.h"
#include "PostprocessorInterface.h"
#include "Receiver.h"
#include <tuple>

InputParameters
FoamPostprocessorBCBase::validParams()
{
  auto params = FoamBCBase::validParams();

  params.addParam<PostprocessorName>("pp_name", "optional postprocessor to be used in BC");
  params.transferParam<Real>(Receiver::validParams(), "default");

  return params;
}

FoamPostprocessorBCBase::FoamPostprocessorBCBase(const InputParameters & params)
  : FoamBCBase(params),
    PostprocessorInterface(this),
    _pp_name((params.isParamSetByUser("pp_name")) ? params.get<PostprocessorName>("pp_name")
                                                  : _name),
    _pp_value(getPostprocessorValueByName(_pp_name))
{
  if (params.isParamSetByUser("pp_name") && params.isParamSetByUser("default"))
    mooseWarning("'pp_name' and 'default' should not both be set. 'default' ignored.");
}

BCInfoTableRow
FoamPostprocessorBCBase::addInfoRow() const
{
  return std::make_tuple(name(),
                         type(),
                         foamVariable(),
                         moosePostprocessor(),
                         Hippo::internal::listFromVector(boundary()));
}
