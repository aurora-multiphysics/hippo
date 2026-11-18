#include "FoamFieldAccessor.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include <processorFvPatch.H>

template <typename T>
FoamFieldAccessor<T>::FoamFieldAccessor(FieldMap & boundary_data,
                                        std::optional<FieldRef> internal_field)
  : _internal_field(internal_field)
{
  for (auto & [name, field] : boundary_data)
  {
    _boundary_data.emplace(name, std::ref(field));
  }
}

template <typename T>
FoamFieldAccessor<T>::FoamFieldAccessor(const FieldRefMap & boundary_data,
                                        std::optional<FieldRef> internal_field)
  : _internal_field(internal_field), _boundary_data(boundary_data)
{
}

template <typename T>
FoamFieldAccessor<T>::FoamFieldAccessor(Field & internal_field)
  : _internal_field(std::ref(internal_field))
{
}

template <typename T>
FoamFieldAccessor<T>::FoamFieldAccessor(Foam::VolField<T> & vol_field)
  : _internal_field(std::ref(vol_field.primitiveFieldRef()))
{
  auto & boundaries = vol_field.boundaryFieldRef();
  for (auto & patch : boundaries)
  {
    // Partition interfaces are not physical transfer boundaries.
    if (Foam::isA<Foam::processorFvPatch>(patch.patch()))
      continue;
    _boundary_data.emplace(patch.patch().name(), std::ref(static_cast<Field &>(patch)));
  }
}

template <typename T>
typename FoamFieldAccessor<T>::Field &
FoamFieldAccessor<T>::boundaryField(const BoundaryName & boundary)
{
  if (_boundary_data.find(boundary) == _boundary_data.end())
    mooseError("Boundary '", boundary, "' not found.");
  return _boundary_data.at(boundary).get();
}

template <typename T>
typename FoamFieldAccessor<T>::Field &
FoamFieldAccessor<T>::internalField()
{
  if (!_internal_field)
    mooseError("No internal field registered in field accessor.");
  return _internal_field->get();
}

template <typename T>
std::vector<std::string>
FoamFieldAccessor<T>::boundaryNames() const
{
  std::vector<std::string> names;
  names.reserve(_boundary_data.size());
  for (const auto & entry : _boundary_data)
    names.push_back(entry.first);
  return names;
}

template class FoamFieldAccessor<Foam::scalar>;
template class FoamFieldAccessor<Foam::vector>;
template class FoamFieldAccessor<Foam::tensor>;
