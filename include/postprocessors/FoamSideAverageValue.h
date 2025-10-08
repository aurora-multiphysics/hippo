#include "FoamPostprocessorBase.h"

class FoamSideAverageValue : public FoamPostprocessorBase
{
  static InputParameters validParams();

  FoamSideAverageValue(const InputParameters & params);

  virtual void execute() override;

  virtual void initialize() override;

  virtual void finalize() override;

  virtual void threadJoin(const UserObject & uo) override;

  virtual PostprocessorValue getValue() const override;

protected:
  Real _value;

  std::string _foam_scalar;
};
