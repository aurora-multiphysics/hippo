#pragma once

#include "InputParameters.h"
#include "MooseEnum.h"
#include "MultiAppFoamTransfer.h"
#include "libmesh/mesh_function.h"
#include <memory>

class MultiApplibMeshToFoamTransfer : public MultiAppFoamTransfer
{
public:
  static InputParameters validParams();

  MultiApplibMeshToFoamTransfer(const InputParameters & params);

  void execute() override;

protected:
  template <typename T>
  void buildTransferMapping();

  template <typename T>
  void transferVariables();

  template <typename T>
  T FoamOutOfRangeValue();

private:
  std::unique_ptr<libMesh::MeshFunction> _mesh_function;
  /// Vector of source variable names to be transferred
  const VariableName & _foam_var_name;
  /// Vector of destination variable names to transfer to
  const VariableName & _libmesh_var_name;
  /// number of points
  unsigned _n_points{0};

  std::map<processor_id_type, std::vector<Point>> _outgoing_points;

  bool _mapping_built{false};
};
