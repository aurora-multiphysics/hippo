#pragma once

#include "InputParameters.h"
#include "MooseObjectAction.h"
#include "FoamProblem.h"

template <typename T>
inline void
copyParamFromParam(InputParameters & dst, const InputParameters & src, const std::string & name_in)
{
  if (src.isParamValid(name_in))
    dst.set<T>(name_in) = src.get<T>(name_in);
}

class AddFoamBCAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddFoamBCAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // Create AuxVariable associated with new-style BCs
  void createAuxVariable();
};
