#include "fvCFD_moose.h"
#include "MultiApplibMeshToFoamTransfer.h"
#include "FoamProblem.h"
#include "LibMeshShapeFunctionEvaluator.h"
#include "LibMeshNearestLocationEvaluator.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MultiApp.h"
#include "Registry.h"
#include <memory>

registerMooseObject("hippoApp", MultiApplibMeshToFoamTransfer);

InputParameters
MultiApplibMeshToFoamTransfer::validParams()
{
  InputParameters params = MultiAppFoamTransfer::validParams();
  params.addParam<std::vector<std::string>>(
      "foam_boundaries", std::vector<std::string>{}, "Native OpenFOAM destination patches.");
  params.addRequiredParam<VariableName>("foam_variable",
                                        "Registered OpenFOAM destination container name");
  params.addRequiredParam<VariableName>("libmesh_variable", "Source libMesh scalar field variable");
  params.addParam<MooseEnum>("evaluation_method",
                             MooseEnum("shape_function nearest_location", "shape_function"),
                             "Source field evaluation method.");
  params.addParam<MooseEnum>(
      "source_type",
      MooseEnum("nodes centroids variable_default", "variable_default"),
      "Sampling for nearest_location: nodes, centroids (vertex averages), or "
      "variable_default (nodes for nodal variables, centroids otherwise). "
      "Source geometry and destination mappings are cached and must remain fixed.");
  params.addRangeCheckedParam<unsigned int>(
      "num_nearest_points",
      1,
      "num_nearest_points > 0",
      "Number of globally nearest samples to average equally. Distance ties are resolved by "
      "global entity ID, independently of source rank ownership.");
  return params;
}

MultiApplibMeshToFoamTransfer::MultiApplibMeshToFoamTransfer(const InputParameters & params)
  : MultiAppFoamTransfer(params),
    _foam_var_name(getParam<VariableName>("foam_variable")),
    _libmesh_var_name(getParam<VariableName>("libmesh_variable")),
    _outgoing_points()
{
  if (getParam<MooseEnum>("evaluation_method") != "nearest_location" &&
      isParamSetByUser("source_type"))
    paramError("source_type", "Only valid with evaluation_method = nearest_location.");
  if (getParam<MooseEnum>("evaluation_method") != "nearest_location" &&
      isParamSetByUser("num_nearest_points"))
    paramError("num_nearest_points", "Only valid with evaluation_method = nearest_location.");
}

void
MultiApplibMeshToFoamTransfer::buildTransferMapping()
{
  _outgoing_points.clear();

  auto & foam_problem = foamProblem(getActiveToProblem());
  const auto & patches = foam_problem.mesh().fvMesh().boundary();
  const auto & foam_boundaries = getParam<std::vector<std::string>>("foam_boundaries");

  for (const auto & boundary : foam_boundaries)
  {
    if (_outgoing_points.find(boundary) != _outgoing_points.end())
      mooseError("Patch '", boundary, "' is specified more than once.");

    const auto patch_id = patches.findIndex(boundary);
    if (patch_id < 0)
      mooseError("Unknown OpenFOAM patch '", boundary, "'.");

    const auto & patch = patches[patch_id];
    const auto & field = _boundary_transfer->get().boundaryField(boundary);
    if (field.size() != patch.size())
      mooseError("Destination field size does not match OpenFOAM patch '", boundary, "'.");

    auto & points = _outgoing_points[boundary];
    points.reserve(patch.size());
    for (const auto & point : patch.Cf())
    {
      libMesh::Point point_in_target_frame;
      for (auto d = 0; d < Foam::vector::nComponents; ++d)
        point_in_target_frame(d) = point[d];
      points.push_back(mapPointToActiveSourceFrame(point_in_target_frame));
    }
  }
}

void
MultiApplibMeshToFoamTransfer::transferVariables()
{
  const std::vector<Point> empty_points;
  Foam::Field<Foam::scalar> empty_field;
  for (const auto & boundary : getParam<std::vector<std::string>>("foam_boundaries"))
  {
    const auto & points = _boundary_transfer ? _outgoing_points.at(boundary) : empty_points;
    auto & field =
        _boundary_transfer ? _boundary_transfer->get().boundaryField(boundary) : empty_field;
    // Every source rank participates, including those without a destination app.
    _source_evaluator->evaluate(points, field);
  }
}

void
MultiApplibMeshToFoamTransfer::execute()
{
  getAppInfo();

  if (_current_direction != TO_MULTIAPP)
    mooseError("MultiApplibMeshToFoamTransfer currently supports only parent-to-child transfers.");

  auto multi_app = getToMultiApp();

  // TODO: Support all MultiApp instances and directions with per-pair mappings before merge.
  setActiveFromProblem(multi_app->problemBase(), 0);
  const bool has_destination = multi_app->hasLocalApp(0);
  if (has_destination)
    setActiveToProblem(multi_app->appProblemBase(0), 0);

  if (!_mapping_built)
  {
    // Every parent rank can own source elements needed by a destination rank.
    if (getParam<MooseEnum>("evaluation_method") == "nearest_location")
    {
      _source_evaluator = std::make_unique<LibMeshNearestLocationEvaluator<Foam::scalar>>(
          getActiveFromProblem(),
          _libmesh_var_name,
          _displaced_source_mesh,
          comm(),
          getParam<MooseEnum>("source_type"),
          getParam<unsigned int>("num_nearest_points"));
    }
    else
      _source_evaluator = std::make_unique<LibMeshShapeFunctionEvaluator<Foam::scalar>>(
          getActiveFromProblem(), _libmesh_var_name, _displaced_source_mesh, comm());
    if (has_destination)
    {
      FoamProblem & foam_problem = foamProblem(getActiveToProblem());
      _boundary_transfer = std::ref(
          foam_problem.getBoundaryTransfer<Foam::scalar>(_foam_var_name, TransferQuantity::Value));
      buildTransferMapping();
    }
    _mapping_built = true;
  }

  transferVariables();
}
