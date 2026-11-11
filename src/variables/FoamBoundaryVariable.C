#include "FoamBoundaryVariable.h"
#include "FoamProblem.h"
#include "MooseError.h"
#include "Registry.h"

#include <type_traits>

registerMooseObject("hippoApp", FoamBoundaryScalarVariable);
registerMooseObject("hippoApp", FoamBoundaryVectorVariable);

template <typename T>
InputParameters
FoamBoundaryVariable<T>::validParams()
{
  auto params = FoamFieldBase::validParams();
  params.addClassDescription("Owns boundary fields that receive direct transfers into OpenFOAM.");
  params.addRequiredParam<std::vector<BoundaryName>>("boundary", "Native OpenFOAM patch names.");
  params.set<bool>("_create_aux_variable") = false;
  constexpr unsigned int components = std::is_same_v<T, Foam::scalar> ? 1 : 3;
  params.set<std::vector<Real>>("initial_condition") = std::vector<Real>(components, 0.0);
  return params;
}

template <typename T>
FoamBoundaryVariable<T>::FoamBoundaryVariable(const InputParameters & params)
  : FoamFieldBase(params)
{
  const auto & boundaries = getParam<std::vector<BoundaryName>>("boundary");
  if (boundaries.empty())
    mooseError("At least one native OpenFOAM patch must be specified.");

  const auto & initial = getParam<std::vector<Real>>("initial_condition");
  constexpr unsigned int components = std::is_same_v<T, Foam::scalar> ? 1 : 3;
  if (initial.size() != components)
    mooseError("Expected ", components, " component(s) in initial conditions.");

  const T initial_value = [&]() -> T
  {
    if constexpr (std::is_same_v<T, Foam::scalar>)
      return initial[0];
    else
      return Foam::vector(initial[0], initial[1], initial[2]);
  }();

  FoamProblem & problem = getFoamProblem();
  if (problem.hasBoundaryTransfer<T>(name(), TransferQuantity::Value))
    mooseError("Boundary transfer '", name(), "' is already registered for this field type.");

  const auto & patches = getFvMesh().boundary();
  for (const auto & boundary : boundaries)
  {
    if (_data.find(boundary) != _data.end())
      mooseError("Patch '", boundary, "' is specified more than once.");

    const auto patch_id = patches.findIndex(boundary);
    if (patch_id < 0)
      mooseError("Unknown OpenFOAM patch '", boundary, "'.");

    _data.emplace(boundary, Foam::Field<T>(patches[patch_id].size(), initial_value));
  }

  // Register only after all owned fields have been initialized and validated.
  problem.addBoundaryTransfer<T>(name(), TransferQuantity::Value, _data).lock();
}

template class FoamBoundaryVariable<Foam::scalar>;
template class FoamBoundaryVariable<Foam::vector>;
