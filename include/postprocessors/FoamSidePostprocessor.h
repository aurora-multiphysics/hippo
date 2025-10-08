#include "BlockRestrictable.h"
#include "FoamPostprocessorBase.h"
#include "InputParameters.h"

// When FoamMesh is updated  use BoundaryRestrictable when Foammesh is refactored
class FoamSidePostprocessor : public FoamPostprocessorBase, public BlockRestrictable
{
public:
  static InputParameters validParams();

  FoamSidePostprocessor(const InputParameters & params);
};
