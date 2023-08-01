#if 0
#include "icoFoamProblem.h"
#include "ArgsUtil.h"

registerMooseObject("hippoApp", icoFoamProblem);

InputParameters
icoFoamProblem::validParams() {
  auto params =  FoamProblemBase::validParams();
  std::vector<std::string> empty_vec;
  params.addParam<std::vector<std::string>>("foam_args",empty_vec,"List of arguments to be passed to openFoam solver");
  return params;
}

icoFoamProblem::icoFoamProblem(InputParameters const &params)
  : FoamProblemBase(params)
  , app_(env_)
  {

  }

void
icoFoamProblem::externalSolve()
{
  app_.run(env_);
}
#endif
