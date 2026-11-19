#include "FoamVariable.h"
#include "FoamProblem.h"
#include "FoamMesh.h"
#include "hippoUtils.h"
#include <algorithm>
#include <type_traits>

registerMooseObject("hippoApp", FoamScalarVariable);
registerMooseObject("hippoApp", FoamVectorVariable);

template <typename T>
InputParameters
FoamVariable<T>::validParams()
{
  auto params = FoamFieldBase::validParams();
  params.addClassDescription("Wraps an existing OpenFOAM field or owns boundary/internal buffers.");
  params.addParam<std::string>("foam_variable", "Existing OpenFOAM volume field to wrap.");

  // Parameters which decide storage allocation
  params.addParam<std::vector<BoundaryName>>(
      "boundary", std::vector<BoundaryName>{}, "Physical patches to allocate.");
  params.addParam<bool>("volume", false, "Allocate internal storage for local OpenFOAM cells.");

  params.addParam<bool>("create_aux_variable",
                        "Mirror boundary values into a scalar AuxVariable. Defaults to true for "
                        "wrapped scalar fields and false otherwise.");

  constexpr unsigned int components = Foam::pTraits<T>::nComponents;
  params.set<std::vector<Real>>("initial_condition") = std::vector<Real>(components, 0.0);

  // Required to statically check whether a variable can be mirrored
  params.addPrivateParam<bool>("_scalar_foam_variable", std::is_same_v<T, Foam::scalar>);
  return params;
}

template <typename T>
FoamVariable<T>::FoamVariable(const InputParameters & params)
  : FoamFieldBase(params),
    _foam_variable(isParamValid("foam_variable") ? getParam<std::string>("foam_variable") : ""),
    _create_aux_variable(mirrorEnabled(params))
{
  if (isParamValid("foam_variable"))
  {
    if (isParamSetByUser("boundary") || isParamSetByUser("volume"))
    {
      paramError("foam_variable", "boundary and volume only apply to owned storage.");
    }
    // If foam_variable is present, Foam-based storage doesn't need to be allocated
    return;
  }

  // If object doesn't reference existing VolField, it must store boundary or internal data
  const auto & boundaries = getParam<std::vector<BoundaryName>>("boundary");
  if (boundaries.empty() && !getParam<bool>("volume"))
    paramError("boundary", "Select at least one boundary or enable volume storage.");

  // Check and get initial conditions
  const auto & initial = getParam<std::vector<Real>>("initial_condition");
  constexpr unsigned int components = Foam::pTraits<T>::nComponents;
  if (initial.size() != components)
    paramError("initial_condition", "Expected ", components, " component(s).");
  const T value = Hippo::internal::fromComponents<T>(initial);

  // Validate specified boundaries then allocate and set fields
  const auto & patches = getFvMesh().boundary();
  for (const auto & boundary : boundaries)
  {
    if (_boundary_data.count(boundary))
      paramError("boundary", "Patch '", boundary, "' is specified more than once.");

    const auto id = patches.findIndex(boundary);
    if (id < 0)
      paramError("boundary", "Unknown OpenFOAM patch '", boundary, "'.");

    _boundary_data.emplace(boundary, Foam::Field<T>(patches[id].size(), value));
  }

  // Allocate internal storage if requested.
  std::optional<typename FoamFieldAccessor<T>::FieldRef> internal;
  if (getParam<bool>("volume"))
  {
    _internal_data.emplace(getFvMesh().nCells(), value);
    internal = std::ref(*_internal_data);
  }
  getFoamProblem().registerField<T>(
      name(), std::make_unique<FoamFieldAccessor<T>>(_boundary_data, internal));
}

template <typename T>
void
FoamVariable<T>::initialSetup()
{
  if (isParamValid("foam_variable"))
    getFoamProblem().registerVolFieldVariable<T>(name(), _foam_variable);
}

template <typename T>
FoamFieldAccessor<T> &
FoamVariable<T>::getFieldAccessor()
{
  return getFoamProblem().getFieldAccessor<T>(name());
}

template <typename T>
void
FoamVariable<T>::syncVariable()
{
  // Boundary mirroring is only required when an AuxVariable was created.
  if (!_create_aux_variable)
    return;

  // Mirroring only valid for scalar fields
  if constexpr (std::is_same_v<T, Foam::scalar>)
  {
    auto & variable = getFoamProblem().getVariable(getParam<THREAD_ID>("_tid"), name());
    auto & accessor = getFieldAccessor();
    const auto boundaries = accessor.boundaryNames();
    for (const auto subdomain : getFoamMesh().getSubdomainList())
    {
      const auto & boundary = getFoamMesh().getSubdomainName(subdomain);
      if (std::find(boundaries.begin(), boundaries.end(), boundary) != boundaries.end())
        Hippo::internal::copyFieldFoamToMoose(
            getFoamMesh(), accessor.boundaryField(boundary), variable, subdomain);
    }
    variable.sys().solution().close();
  }
}

template class FoamVariable<Foam::scalar>;
template class FoamVariable<Foam::vector>;
