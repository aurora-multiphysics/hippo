#pragma once

#include "fvCFD_moose.h"
#include "TransferContainerBase.h"
#include <functional>
#include <type_traits>
#include "MooseTypes.h"

/// Supported native field types and their registry keys.
template <typename T>
struct TransferFieldTraits
{
  static_assert(std::is_same_v<T, Foam::scalar> || std::is_same_v<T, Foam::vector> ||
                    std::is_same_v<T, Foam::tensor>,
                "Unsupported OpenFOAM transfer field type.");
  static constexpr TransferFieldType value =
      std::is_same_v<T, Foam::scalar>   ? TransferFieldType::Scalar
      : std::is_same_v<T, Foam::vector> ? TransferFieldType::Vector
                                        : TransferFieldType::Tensor;
};

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
