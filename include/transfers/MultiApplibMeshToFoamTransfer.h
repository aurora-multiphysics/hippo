#pragma once

#include "InputParameters.h"
#include "MultiAppFoamTransfer.h"
#include "BoundaryTransferContainer.h"
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
  void buildTransferMapping();

  void transferVariables();

private:
  std::unique_ptr<LibMeshTransferEvaluator<Foam::scalar>> _source_evaluator;
  /// Registered destination container name
  const VariableName & _foam_var_name;
  /// Source libMesh variable name
  const VariableName & _libmesh_var_name;
  /// Borrowed destination container, bound once the active problems are selected
  std::optional<std::reference_wrapper<BoundaryTransferContainer<Foam::scalar>>> _boundary_transfer;

  std::map<std::string, std::vector<Point>> _outgoing_points;

  bool _mapping_built{false};
};
