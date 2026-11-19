#pragma once

#include "FoamFieldAccessorBase.h"
#include "fvCFD_moose.h"
#include "MooseTypes.h"

#include <functional>
#include <map>
#include <optional>
#include <vector>

/**
 * Borrow writable internal and boundary values without owning their storage.
 *
 * Internal storage and named boundaries are independently optional. References are fixed at
 * construction; their owners must outlive the accessor and must not replace the referenced
 * field objects.
 */
template <typename T>
class FoamFieldAccessor : public FoamFieldAccessorBase
{
public:
  using Field = Foam::Field<T>;
  using FieldRef = std::reference_wrapper<Field>;
  using FieldMap = std::map<BoundaryName, Field>;
  using FieldRefMap = std::map<BoundaryName, FieldRef>;

  /// Borrow fields owned by the caller's map, with optional internal storage.
  explicit FoamFieldAccessor(FieldMap & boundary_data,
                             std::optional<FieldRef> internal_field = std::nullopt);
  /// Copy references, not values. An empty map exposes no boundaries; defaults expose no storage.
  explicit FoamFieldAccessor(const FieldRefMap & boundary_data = {},
                             std::optional<FieldRef> internal_field = std::nullopt);

  /// Borrow internal storage alone, without exposing any boundaries.
  explicit FoamFieldAccessor(Field & internal_field);
  /// Borrow internal storage and non-processor boundary values of a volume field.
  explicit FoamFieldAccessor(Foam::VolField<T> & vol_field);

  /// Whether internal storage was supplied, including a valid zero-length field.
  bool hasInternalField() const { return _internal_field.has_value(); }

  /// Return writable internal values; report an error if no internal field was supplied.
  Field & internalField();
  /// Return values for an exposed patch name; report an error if that name is absent.
  Field & boundaryField(const BoundaryName & boundary);

  /// Return exposed patch names in sorted order, including patches with zero local faces.
  std::vector<std::string> boundaryNames() const;

private:
  std::optional<FieldRef> _internal_field;
  FieldRefMap _boundary_data;
};
