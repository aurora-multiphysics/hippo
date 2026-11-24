#pragma once

#include "FoamFieldAccessor.h"
#include "MooseError.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

/**
 * Own one accessor per exposing object's unique name.
 *
 * Separate accessors may borrow the same storage. Underlying OpenFOAM field names
 * are not reserved automatically; storage remains owned by its original owner.
 */
class FoamFieldRegistry
{
public:
  /// Check a registered object name, without creating an accessor.
  bool contains(const std::string & name) const
  {
    return _accessors.find(name) != _accessors.end();
  }

  /// Take ownership of a supplied accessor; an already registered name is an error.
  template <typename T>
  FoamFieldAccessor<T> & add(const std::string & name,
                             std::unique_ptr<FoamFieldAccessor<T>> accessor)
  {
    if (contains(name))
      mooseError("Field accessor '", name, "' is already registered.");
    auto & result = *accessor;
    _accessors.emplace(name, std::move(accessor));
    return result;
  }

  /// Retrieve by registered object name; missing names and incompatible types are errors.
  template <typename T>
  FoamFieldAccessor<T> & getFieldAccessor(const std::string & name)
  {
    const auto it = _accessors.find(name);
    if (it == _accessors.end())
      mooseError("Field accessor '", name, "' is not registered.");
    auto * accessor = dynamic_cast<FoamFieldAccessor<T> *>(it->second.get());
    if (!accessor)
      mooseError("Field accessor '", name, "' has an incompatible field type.");
    return *accessor;
  }

private:
  /// Own the accessors, not the field storage they reference.
  std::map<std::string, std::unique_ptr<FoamFieldAccessorBase>> _accessors;
};
