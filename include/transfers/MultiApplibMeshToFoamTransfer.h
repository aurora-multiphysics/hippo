#pragma once

#include "InputParameters.h"
#include "MultiAppFoamTransfer.h"
#include "BoundaryTransferContainer.h"
#include "libmesh/mesh_function.h"
#include <functional>
#include <map>
#include <memory>
#include <optional>

class MultiApplibMeshToFoamTransfer : public MultiAppFoamTransfer
{
public:
  static InputParameters validParams();

  MultiApplibMeshToFoamTransfer(const InputParameters & params);

  void execute() override;

protected:
  template <typename T>
  void prepareSourceEvaluator();

  template <typename T>
  void buildTransferMapping();

  template <typename T>
  void transferVariables();

  template <typename T>
  T FoamOutOfRangeValue();

private:
  std::unique_ptr<libMesh::MeshFunction> _mesh_function;
  /// Registered destination container name
  const VariableName & _foam_var_name;
  /// Source libMesh variable name
  const VariableName & _libmesh_var_name;
  /// Borrowed destination container, bound once the active problems are selected
  std::optional<std::reference_wrapper<BoundaryTransferContainer<Foam::scalar>>> _boundary_transfer;

  std::map<std::string, std::map<processor_id_type, std::vector<Point>>> _outgoing_points;

  bool _mapping_built{false};
};
