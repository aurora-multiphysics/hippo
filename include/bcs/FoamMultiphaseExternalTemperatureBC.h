#pragma once

#include "FoamVariableBCBase.h"

namespace Foam
{
class phaseSystem;
}

class FoamMultiphaseExternalTemperatureBC : public FoamVariableBCBase
{
public:
  static InputParameters validParams();
  explicit FoamMultiphaseExternalTemperatureBC(const InputParameters &);
  std::vector<std::string> foamVariables() const override;
  void initialSetup() override;
  void imposeBoundaryCondition(bool initialisation = false) override;

protected:
  void constructFoamPatch(Foam::label patch_id) override;
  Foam::phaseSystem & _phases;
};
