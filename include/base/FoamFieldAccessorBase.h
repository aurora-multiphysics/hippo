#pragma once

/// Allows the registry to own accessors of different field types through a common base.
/// The referenced OpenFOAM storage remains owned by its provider.
class FoamFieldAccessorBase
{
public:
  virtual ~FoamFieldAccessorBase() = default;
  /// Prevent assignment from replacing an accessor's established field references.
  FoamFieldAccessorBase & operator=(const FoamFieldAccessorBase &) = delete;
};
