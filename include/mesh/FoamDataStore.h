#pragma once

#include <DataIO.h>

#include <fvCFD_moose.h>

// This function extracts the keys associated with fields of type T from the
// mesh object registry. Note for some fields, the field.name() and the
// key are not the same. *strict* indicates whether types derived from T are
// collected
template <typename T, bool strict>
inline std::vector<Foam::string>
getFieldkeys(const Foam::fvMesh & mesh)
{
  std::vector<Foam::string> fieldKeyList;
  for (auto key : mesh.toc())
  {
    if (mesh.foundObject<T>(key))
    {
      auto & field = mesh.lookupObjectRef<T>(key);
      if (!field.isOldTime() && (Foam::isType<T>(field) || (Foam::isA<T>(field) && !strict)))
      {
        fieldKeyList.push_back(key);
      }
    }
  }

  return fieldKeyList;
}

template <class Type, class GeomMesh>
inline void
readBoundary([[maybe_unused]] istream & stream,
             [[maybe_unused]] Foam::DimensionedField<Type, GeomMesh> & field)
{
}

template <typename GeoField>
inline void
readBoundary(istream & stream, GeoField & field)
{
  for (auto & bField : field.boundaryFieldRef())
  {
    std::vector<typename GeoField::value_type> data(bField.size());
    loadHelper(stream, data, nullptr);
    std::copy(data.begin(), data.end(), bField.begin());
  }
}

// readField for GeometricFields and DimensionedFields
template <typename GeoField>
inline void
readField(std::istream & stream, GeoField & field)
{

  std::vector<typename GeoField::value_type> internal_data(field.size());
  loadHelper(stream, internal_data, nullptr);

  for (auto i = 0lu; i < internal_data.size(); ++i)
  {
    field.primitiveFieldRef()[i] = internal_data[i];
  }

  readBoundary(stream, field);
}

template <>
inline void
readField(std::istream & stream, Foam::uniformDimensionedScalarField & field)
{
  Foam::scalar value;
  loadHelper(stream, value, nullptr);
  field.value() = value;
}

template <class Type, class GeomMesh>
inline void
writeBoundary(ostream & stream, const Foam::DimensionedField<Type, GeomMesh> & field)
{
  (void)stream;
  (void)field;
}

template <class GeoField>
inline void
writeBoundary(ostream & stream, const GeoField & field)
{
  for (auto & bField : field.boundaryField())
  {
    std::vector<typename GeoField::value_type> data(bField.size());
    std::copy(bField.begin(), bField.end(), data.begin());
    storeHelper(stream, data, nullptr);
  }
}

// writeField for GeometricFields and DimensionedFields
template <typename GeoField>
inline void
writeField(ostream & stream, const GeoField & field)
{
  std::vector<typename GeoField::value_type> internal_field(field.primitiveField().size());
  std::copy(field.primitiveField().begin(), field.primitiveField().end(), internal_field.begin());

  storeHelper(stream, internal_field, nullptr);

  writeBoundary(stream, field);
}

// writeField for UniformDimensionedFields
template <typename Type>
inline void
writeField(ostream & stream, const Foam::UniformDimensionedField<Type> & field)
{
  storeHelper(stream, field.value(), nullptr);
}

// Generic function for serialising any field and its old times
template <typename T>
inline void
dataStoreField(std::ostream & stream, const Foam::string & name, T & field, void * context)
{
  auto nOldTimes{field.nOldTimes(false)};
  storeHelper(stream, nOldTimes, context);

  std::string field_name{name};
  storeHelper(stream, field_name, nullptr);
  writeField(stream, field);

  for (int n = 1; n <= nOldTimes; ++n)
  {
    writeField(stream, field.oldTime(n));
  }
}

// Generic function for deserialising any field and its old times
template <typename T>
inline void
dataLoadField(std::istream & stream, Foam::fvMesh & foam_mesh)
{

  Foam::label nOldTimes;
  loadHelper(stream, nOldTimes, nullptr);

  std::string field_name;
  loadHelper(stream, field_name, nullptr);
  auto & field = foam_mesh.lookupObjectRef<T>(field_name);
  readField(stream, field);

  for (int nOld = 1; nOld <= nOldTimes; ++nOld)
  {
    auto & old_field = field.oldTimeRef(nOld);
    readField(stream, old_field);
  }
}

// serialises all fields of type T
template <typename T, bool strict>
inline void
storeFields(std::ostream & stream, const Foam::fvMesh & mesh, void * context)
{
  const auto cur_fields{getFieldkeys<T, strict>(mesh)};
  auto nFields{static_cast<int>(cur_fields.size())};

  storeHelper(stream, nFields, context);
  for (auto & key : cur_fields)
  {
    auto & field = mesh.lookupObjectRef<T>(key);

    dataStoreField<T>(stream, key, field, context);
  }
}

// These structs statically determine whether a class is a geometric type
// returns the first if type doesn't match a GeometricField and true if it
// does
template <typename>
struct is_geometric_field : std::false_type
{
};

template <typename Type, template <class> class Patch, typename Mesh>
struct is_geometric_field<Foam::GeometricField<Type, Patch, Mesh>> : std::true_type
{
};

template <typename T>
void
removeOldTime(Foam::fvMesh & mesh, T & field)
{
  // This is required for Hippo to behave the exact same as OpenFOAM when using
  // fixed-point iteration. The differences only affect the fvc::ddt calls on the first
  // time step. In OpenFOAM, on the first timestep fvc::ddt calls return 0. However,
  // on the second fixed-point they don't unless the old time base field is cleared, but
  // this results in an internal OpenFOAM error for some time schemes.
  // Schemes known to work:
  //   - Euler (implicit)
  // Schemes known not to work
  //   - Crank-Nicolson
  auto scheme = Foam::fv::ddtScheme<typename T::cmptType>::New(
                    mesh, mesh.schemes().ddt("ddt(" + field.name() + ')'))
                    ->type();

  if (scheme == "Euler")
  {
    // Only geometric fields have a base field
    if constexpr (is_geometric_field<T>::value)
    {
      // otbf is set in the setBase functions of the OldTimeField. This is mirrored here
      // in order to null it.
      auto & otbf = const_cast<typename T::Base::OldTime &>(Foam::OldTimeBaseFieldType<T>()(field));
      otbf.clearOldTimes();
      otbf.nullOldestTime();
    }
    field.clearOldTimes();
  }
}

template <typename T>
inline void
loadFields(std::istream & stream, Foam::fvMesh & mesh, void * context)
{
  int nFields{};
  loadHelper(stream, nFields, context);
  for (int i = 0; i < nFields; ++i)
  {
    dataLoadField<T>(stream, mesh);
  }

  for (auto & field : mesh.curFields<T>())
  {
    // Remove fields that haven't been stored. Important for subcycling to prevent the old
    // fields being which haven't been stored being used on the first time step.
    if (mesh.time().timeIndex() == 0)
    {
      removeOldTime(mesh, field);
      if (mesh.time().timeIndex() != field.timeIndex())
      {
        mesh.checkOut(field);
      }
    }
  }
}

template <>
inline void
dataStore(std::ostream & stream, const Foam::Time & time, void * context)
{
  auto timeIndex = time.timeIndex();
  auto deltaT = time.deltaTValue();
  auto timeValue = time.userTimeValue();

  storeHelper(stream, timeIndex, context);
  storeHelper(stream, deltaT, context);
  storeHelper(stream, timeValue, context);
}

template <>
inline void
dataLoad(std::istream & stream, Foam::Time & time, void * context)
{
  Foam::label timeIndex;
  Foam::scalar deltaT, timeValue;

  loadHelper(stream, timeIndex, context);
  loadHelper(stream, deltaT, context);
  loadHelper(stream, timeValue, context);

  time.setDeltaTNoAdjust(deltaT);
  // This ensures that the delta0 variable is internally updated before
  // the step allowing variable deltaT to be used
  time++;

  // reset time and time index
  time.setTime(time, timeIndex);
  time.setTime(timeValue, timeIndex);
}

// Main function for storing data called as a result of the
// declareDataRecoverable in FoamMesh
template <>
inline void
dataStore(std::ostream & stream, Foam::fvMesh & mesh, void * context)
{
  storeHelper(stream, mesh.time(), context);

  storeFields<Foam::volScalarField, false>(stream, mesh, context);
  storeFields<Foam::volVectorField, false>(stream, mesh, context);
  storeFields<Foam::volTensorField, false>(stream, mesh, context);
  storeFields<Foam::volSymmTensorField, false>(stream, mesh, context);

  storeFields<Foam::surfaceScalarField, false>(stream, mesh, context);
  storeFields<Foam::surfaceVectorField, false>(stream, mesh, context);
  storeFields<Foam::surfaceTensorField, false>(stream, mesh, context);
  storeFields<Foam::surfaceSymmTensorField, false>(stream, mesh, context);

  storeFields<Foam::DimensionedField<Foam::scalar, Foam::volMesh>, true>(stream, mesh, context);
  storeFields<Foam::DimensionedField<Foam::vector, Foam::volMesh>, true>(stream, mesh, context);
  storeFields<Foam::DimensionedField<Foam::scalar, Foam::surfaceMesh>, true>(stream, mesh, context);
  storeFields<Foam::DimensionedField<Foam::vector, Foam::surfaceMesh>, true>(stream, mesh, context);

  storeFields<Foam::uniformDimensionedScalarField, true>(stream, mesh, context);
}

// Main function for loading data called as a result of the
// declareDataRecoverable in FoamMesh
template <>
inline void
dataLoad(std::istream & stream, Foam::fvMesh & mesh, void * context)
{
  loadHelper(stream, const_cast<Foam::Time &>(mesh.time()), context);

  loadFields<Foam::volScalarField>(stream, mesh, context);
  loadFields<Foam::volVectorField>(stream, mesh, context);
  loadFields<Foam::volTensorField>(stream, mesh, context);
  loadFields<Foam::volSymmTensorField>(stream, mesh, context);

  loadFields<Foam::surfaceScalarField>(stream, mesh, context);
  loadFields<Foam::surfaceVectorField>(stream, mesh, context);
  loadFields<Foam::surfaceTensorField>(stream, mesh, context);
  loadFields<Foam::surfaceSymmTensorField>(stream, mesh, context);

  loadFields<Foam::DimensionedField<Foam::scalar, Foam::volMesh>>(stream, mesh, context);
  loadFields<Foam::DimensionedField<Foam::vector, Foam::volMesh>>(stream, mesh, context);
  loadFields<Foam::DimensionedField<Foam::scalar, Foam::surfaceMesh>>(stream, mesh, context);
  loadFields<Foam::DimensionedField<Foam::vector, Foam::surfaceMesh>>(stream, mesh, context);

  loadFields<Foam::uniformDimensionedScalarField>(stream, mesh, context);
}
