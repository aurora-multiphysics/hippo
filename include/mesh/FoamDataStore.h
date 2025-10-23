#pragma once

#include "DataIO.h"

#include "DimensionedField.H"
#include "GeometricField.H"
#include "fvCFD_moose.h"

#include <algorithm>
#include <fstream>
#include <ostream>
#include <cxxabi.h>
#include <type_traits>

// This method is to help debug issues with serialising and deserialising
// TODO: Remove before merge
template <typename T>
inline void
outputField(std::string fname, const T & field, bool old_fields = true)
{
  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss << field;

  std::ofstream ofs{fname, std::ios::out | std::ios::trunc};
  ofs << field.name() << " " << field.time().userTimeValue() << std::endl;
  ofs << oss.str();

  if (!old_fields)
    return;

  for (int n = 1; n <= field.nOldTimes(); ++n)
  {
    Foam::OStringStream oss_old(Foam::IOstream::ASCII);
    oss_old << field.oldTime(n);
    ofs << field.oldTime(n).name() << " " << field.oldTime(n).time().userTimeValue() << std::endl;
    ofs << oss_old.str();
  }
  ofs.close();
}

// This method extracts the keys associated with fields of type T from the
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
      if (!field.isOldTime() &&
          ((Foam::isType<T>(field) && strict) || (Foam::isA<T>(field) && !strict)))
      {
        fieldKeyList.push_back(key);
        std::cout << "Field: " << key << std::endl;
      }
    }
  }

  return fieldKeyList;
}

template <class Type, class GeomMesh>
inline void
readBoundary(istream & stream, Foam::DimensionedField<Type, GeomMesh> & field)
{
  (void)stream;
  (void)field;
}

template <class Type, template <class> class PatchField, class GeoMesh>
inline void
readBoundary(istream & stream, Foam::GeometricField<Type, PatchField, GeoMesh> & field)
{
  for (auto & bField : field.boundaryFieldRef())
  {
    std::vector<Type> data(bField.size());
    loadHelper(stream, data, nullptr);
    for (auto i = 0lu; i < data.size(); ++i)
      bField[i] = data[i];
  }
}
// readField abstracts the construction of the field allowing the same
// interface to be used for different types. A pair is returned with
// first being the key in mesh.toc() and second being the field.
//
// readField for GeometricFields and DimensionedFields
template <typename Type>
inline Type &
readField(std::istream & stream,
          Foam::fvMesh & mesh,
          Type * parent_field,
          unsigned int old_time_index)
{
  assert(old_time_index == 0 || parent_field);

  std::string field_name;
  loadHelper(stream, field_name, nullptr);
  auto & field = (old_time_index > 0 && parent_field) ? parent_field->oldTimeRef(old_time_index)
                                                      : mesh.lookupObjectRef<Type>(field_name);

  std::vector<typename Type::value_type> internal_data(field.size());
  loadHelper(stream, internal_data, nullptr);

  for (auto i = 0lu; i < internal_data.size(); ++i)
    field.primitiveFieldRef()[i] = internal_data[i];

  readBoundary(stream, field);

  return field;
}

// readField for uniformDimensionedScalarFields
template <>
inline Foam::uniformDimensionedScalarField &
readField(std::istream & stream,
          Foam::fvMesh & mesh,
          Foam::uniformDimensionedScalarField * parent_field,
          unsigned int old_time_index)
{
  std::pair<std::string, Foam::scalar> field_data;
  loadHelper(stream, field_data, nullptr);
  auto & field = mesh.lookupObjectRef<Foam::uniformDimensionedScalarField>(field_data.first);
  field.value() = field_data.second;

  return field;
}

template <class Type, class GeomMesh>
inline void
writeBoundary(ostream & stream, const Foam::DimensionedField<Type, GeomMesh> & field)
{
  (void)stream;
  (void)field;
}

template <class Type, template <class> class PatchField, class GeoMesh>
inline void
writeBoundary(ostream & stream, const Foam::GeometricField<Type, PatchField, GeoMesh> & field)
{
  for (auto & bField : field.boundaryField())
  {
    std::vector<Type> data(bField.size());
    std::copy(bField.begin(), bField.end(), data.begin());
    storeHelper(stream, data, nullptr);
  }
}
// writeField befores a similar role to readField but for the serialisation
// step
//
// writeField for GeometricFields and DimensionedFields
template <typename Type>
inline void
writeField(ostream & stream, const Foam::string & name, const Type & field)
{
  std::vector<typename Type::value_type> internal_field(field.primitiveField().size());
  std::copy(field.primitiveField().begin(), field.primitiveField().end(), internal_field.begin());

  std::string field_name{name};
  storeHelper(stream, field_name, nullptr);
  storeHelper(stream, internal_field, nullptr);

  writeBoundary(stream, field);
}

// writeField for UniformDimensionedFields
template <typename Type>
inline void
writeField(ostream & stream,
           const Foam::string & name,
           const Foam::UniformDimensionedField<Type> & field)
{
  auto store_pair{std::pair(std::string(name), field.value())};
  storeHelper(stream, store_pair, nullptr);
}

// Generic function for serialising any field and its old times
template <typename T>
inline void
dataStoreField(std::ostream & stream, const Foam::string & name, T & field, void * context)
{
  auto nOldTimes{field.nOldTimes(false)};
  storeHelper(stream, nOldTimes, context);
  std::cout << "Serialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL) << " "
            << field.name() << std::endl;

  writeField(stream, name, field);

  auto old_name = name;
  for (int n = 1; n <= nOldTimes; ++n)
  {
    old_name += "_0";
    writeField(stream, old_name, field.oldTime(n));
    std::cout << "  - Serialising "
              << abi::__cxa_demangle(typeid(field.oldTime(n)).name(), NULL, NULL, NULL) << " "
              << field.oldTime(n).name() << std::endl;
  }
}

// Generic function for deserialising any field and its old times
template <typename T>
inline void
dataLoadField(std::istream & stream, Foam::fvMesh & foam_mesh)
{

  Foam::label nOldTimes;
  loadHelper(stream, nOldTimes, nullptr);

  auto & field = readField<T>(stream, foam_mesh, nullptr, 0);

  std::cout << "Deserialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL)
            << " " << field.name() << " timeindex: " << field.timeIndex() << std::endl;

  for (int nOld = 1; nOld <= nOldTimes; ++nOld)
  {
    auto & old_field = readField<T>(stream, foam_mesh, &field, nOld);
    std::cout << "  - Deserialising " << old_field.name() << ". nOld: " << nOld << std::endl;
  }
}

// dataStore for dimensionSets (contains the units of OpenFOAM fields/variables)
template <>
inline void
dataStore(std::ostream & stream, const Foam::dimensionSet & s, void * context)
{
  for (int i = 0; i < 7; ++i)
  {
    auto dim = s[i];
    storeHelper(stream, dim, context);
  }
}

// dataLoad for dimensionedSets
template <>
inline void
dataLoad(std::istream & stream, Foam::dimensionSet & s, void * context)
{
  for (int i = 0; i < 7; ++i)
  {
    loadHelper(stream, s[i], context);
  }
}

// serialises all fields of type T
template <typename T, bool strict>
inline void
storeFields(std::ostream & stream, const Foam::fvMesh & mesh, void * context)
{
  auto && cur_fields{getFieldkeys<T, strict>(mesh)};
  auto nFields{static_cast<int>(cur_fields.size())};

  storeHelper(stream, nFields, context);
  for (auto & key : cur_fields)
  {
    auto & field = mesh.lookupObjectRef<T>(key);

    // TODO: remove before merge
    // outputField(field.name() + "_out.txt", field, false);
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
      std::cout << "Clearing old times " << field.name() << std::endl;
      removeOldTime(mesh, field);

      if (mesh.time().timeIndex() != field.timeIndex())
      {
        std::cout << "Field " << field.name() << " checked out." << std::endl;
        mesh.checkOut(field);
      }
    }

    // TODO: remove before merge
    // outputField(field.name() + "_in.txt", field, false);
  }
}

// Store the Foam::Time class
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

  // TODO: remove before merge
  std::cout << "Saved time:\n"
            << "\ttimeIndex: " << timeIndex << "\n"
            << "\ttimeValue: " << timeValue << "\n"
            << "\tdeltaT: " << deltaT << std::endl;
}

// Load the Foam::Time class
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

  // TODO: remove before merge
  std::cout << "Loaded time:\n"
            << "\ttimeIndex: " << time.timeIndex() << "\n"
            << "\ttimeValue: " << time.userTimeValue() << "\n"
            << "\tdeltaT: " << time.deltaTValue() << std::endl;
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
