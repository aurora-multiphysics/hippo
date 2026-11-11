#include "BoundaryTransferContainer.h"
#include "MooseError.h"
#include "MooseTypes.h"

template <typename T>
BoundaryTransferContainer<T>::BoundaryTransferContainer(FieldMap & data)
{
  for (auto & [name, field] : data)
  {
    _data.emplace(name, std::ref(field));
  }
}

template <typename T>
BoundaryTransferContainer<T>::BoundaryTransferContainer(FieldRefMap & data)
{
  addBoundaries(data);
}

template <typename T>
void
BoundaryTransferContainer<T>::addBoundaries(const FieldRefMap & data)
{
  if (!this->modifiable())
    mooseError("Cannot add boundaries to a locked transfer container.");

  // Validate the entire registration before changing any references.
  for (const auto & entry : data)
  {
    const auto existing = _data.find(entry.first);
    if (existing != _data.end() && &existing->second.get() != &entry.second.get())
      mooseError("Boundary '", entry.first, "' is already registered with a different field.");
  }

  _data.insert(data.begin(), data.end());
}

template <typename T>
typename BoundaryTransferContainer<T>::Field &
BoundaryTransferContainer<T>::boundaryField(const BoundaryName & boundary)
{
  if (_data.find(boundary) == _data.end())
    mooseError("Boundary '", boundary, "' not found.");
  return _data.at(boundary).get();
}

template <typename T>
typename BoundaryTransferContainer<T>::Field &
BoundaryTransferContainer<T>::volumeField()
{
  mooseError("No volume field in boundary transfer container.");
}

template class BoundaryTransferContainer<Foam::scalar>;
template class BoundaryTransferContainer<Foam::vector>;
template class BoundaryTransferContainer<Foam::tensor>;
