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
  for (auto & [name, field] : data)
  {
    _data.emplace(name, field);
  }
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
