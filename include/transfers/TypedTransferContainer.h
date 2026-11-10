#pragma once

#include "fvCFD_moose.h"
#include "TransferContainerBase.h"
#include <functional>
#include "MooseTypes.h"

template <typename T>
class TypedTransferContainer : public TransferContainerBase
{
public:
  using Field = Foam::Field<T>;
  using FieldRef = std::reference_wrapper<Field>;

  TypedTransferContainer() = default;
  virtual Field & boundaryField(const BoundaryName & boundary) = 0;
  virtual Field & volumeField() = 0;
};
