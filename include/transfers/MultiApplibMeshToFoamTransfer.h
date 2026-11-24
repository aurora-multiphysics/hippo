#pragma once

#include "InputParameters.h"
#include "MultiAppFoamTransfer.h"
#include "FoamFieldAccessor.h"
#include "LibMeshTransferEvaluator.h"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class MultiApplibMeshToFoamTransfer : public MultiAppFoamTransfer
{
public:
  static InputParameters validParams();

  MultiApplibMeshToFoamTransfer(const InputParameters & params);

  void execute() override;

protected:
  void validateVolumeDestination();

  void buildTransferMapping();

  void transferVariables();

private:
  std::unique_ptr<LibMeshTransferEvaluator<Foam::scalar>> _source_evaluator;
  /// Registered destination object name
  const VariableName & _foam_var_name;
  /// Source libMesh variable name
  const VariableName & _libmesh_var_name;
  /// Borrowed destination field accessor, bound once the active problems are selected.
  std::optional<std::reference_wrapper<FoamFieldAccessor<Foam::scalar>>> _destination_accessor;
  std::map<std::string, std::vector<Point>> _outgoing_points;
  /// Source-frame cell centres in local OpenFOAM cell order.
  std::vector<Point> _volume_points;

  /// Common evaluation order on all parent ranks, including ranks without the child.
  std::vector<std::string> _boundaries;

  bool _mapping_built{false};
};
