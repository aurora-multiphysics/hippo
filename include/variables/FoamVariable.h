#pragma once

#include "FoamFieldBase.h"
#include "FoamFieldAccessor.h"

/// A named field backed by an existing VolField or by owned boundary/internal buffers.
template <typename T>
class FoamVariable : public FoamFieldBase
{
public:
  static InputParameters validParams();

  explicit FoamVariable(const InputParameters & params);
  /// Register a borrowed VolField after derived constructors have had a chance to create it.
  void initialSetup() override;
  /// Mirror current boundary values into the AuxVariable, if one was created.
  void syncVariable() override;
  /// Return the wrapped OpenFOAM field name, or an empty string for owned storage.
  std::string foamVariable() const override { return _foam_variable; }

  // Keep owned field objects stable because registered accessors borrow them.
  FoamVariable(const FoamVariable &) = delete;
  FoamVariable & operator=(const FoamVariable &) = delete;
  FoamVariable(FoamVariable &&) = delete;
  FoamVariable & operator=(FoamVariable &&) = delete;

protected:
  /// Available after registration, allowing derived variables to update values before mirroring.
  FoamFieldAccessor<T> & getFieldAccessor();
  const std::string _foam_variable;
  const bool _create_aux_variable;

private:
  typename FoamFieldAccessor<T>::FieldMap _boundary_data;
  std::optional<Foam::Field<T>> _internal_data;
};

using FoamScalarVariable = FoamVariable<Foam::scalar>;
using FoamVectorVariable = FoamVariable<Foam::vector>;
