#pragma once

#include "GeneralUserObject.h"
#include "HippoBase.h"
#include "InputParameters.h"
#include "KDTree.h"
#include <fvPatch.H>
#include <memory>
#include <scalarField.H>

class AdjacentCellBulkTemperature : public GeneralUserObject, public HippoBase
{
public:
  static InputParameters validParams();
  AdjacentCellBulkTemperature(const InputParameters & params);

  void execute() override;
  void initialize() override;
  void finalize() override {};

  Real spatialValue(const Point & point) const override;

private:
  const Foam::fvPatch & getFoamPatch(const std::string & boundary);
  void buildKDTree();

  std::vector<int> _mpi_sizes;
  std::vector<int> _mpi_displs;
  const Foam::fvPatch & _foam_patch;
  std::vector<Point> _kd_centres;
  Foam::scalarField _gl_t_adjacent;
  std::unique_ptr<KDTree> _kd_tree;
};
