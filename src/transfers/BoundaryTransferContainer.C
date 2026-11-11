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
  if (!modifiable())
    mooseError("Cannot add boundaries to a locked transfer container.");

  // Validate the entire registration before changing any references.
  for (const auto & entry : data)
    if (_data.find(entry.first) != _data.end())
      mooseError("Boundary '", entry.first, "' is already registered in the transfer container.");

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
