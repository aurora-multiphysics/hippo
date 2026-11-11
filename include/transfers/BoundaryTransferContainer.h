#pragma once

#include "TypedTransferContainer.h"
#include "fvCFD_moose.h"
#include "MooseTypes.h"
#include <map>

template <typename T>
class BoundaryTransferContainer : public TypedTransferContainer<T>
{
public:
  using Field = typename TypedTransferContainer<T>::Field;
  using FieldRef = typename TypedTransferContainer<T>::FieldRef;
  using FieldMap = std::map<BoundaryName, Field>;
  using FieldRefMap = std::map<BoundaryName, FieldRef>;

  explicit BoundaryTransferContainer(FieldMap & data);
  explicit BoundaryTransferContainer(FieldRefMap & data);

  /// Add borrowed fields; their owners must outlive this container.
  void addBoundaries(const FieldRefMap & data);

  Field & boundaryField(const BoundaryName & boundary) override;
  Field & volumeField() override;

private:
  FieldRefMap _data;
};
