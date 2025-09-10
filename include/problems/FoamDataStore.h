#pragma once

#include "DataIO.h"

#include "fvCFD_moose.h"

#include <fstream>
#include <ostream>
#include <cxxabi.h>

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

// readField abstracts the construction of the field allowing the same
// interface to be used for different types. A pair is returned with
// first being the key in mesh.toc() and second being the field.
//
// readField for GeometricFields and DimensionedFields
template <typename Type>
inline std::pair<std::string, Type>
readField(std::istream & stream, const Foam::fvMesh & mesh)
{
  std::pair<std::string, std::string> field_data;

  loadHelper(stream, field_data, nullptr);
  Foam::IStringStream iss{Foam::string(field_data.second), Foam::IOstream::ASCII};
  return std::pair(
      field_data.first,
      Type{
          Foam::IOobject{
              field_data.first, mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE, false},
          mesh,
          Foam::dictionary(iss),
      });
}

// readField for uniformDimensionedScalarFields
template <>
inline std::pair<std::string, Foam::uniformDimensionedScalarField>
readField(std::istream & stream, const Foam::fvMesh & mesh)
{
  Foam::dimensionSet dimSet{Foam::dimless};
  loadHelper(stream, dimSet, nullptr);
  std::pair<std::string, Foam::scalar> field_data;
  loadHelper(stream, field_data, nullptr);

  return std::pair(
      field_data.first,
      Foam::uniformDimensionedScalarField{
          Foam::IOobject{
              field_data.first, mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE, false},
          Foam::dimensioned<Foam::scalar>(dimSet, field_data.second),
      });
}

// writeField befores a similar role to readField but for the serialisation
// step
//
// writeField for GeometricFields and DimensionedFields
template <typename Type>
inline void
writeField(ostream & stream, const Foam::string & name, const Type & field)
{
  // ensure that data is written to at least double precision
  auto precision = std::max(field.time().controlDict().lookupOrDefault("writePrecision", 17), 17);

  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss.precision(precision);
  oss << field;
  std::pair store_pair{std::string(name), std::string(oss.str())};
  storeHelper(stream, store_pair, nullptr);
}

// writeField for UniformDimensionedFields
template <typename Type>
inline void
writeField(ostream & stream,
           const Foam::string & name,
           const Foam::UniformDimensionedField<Type> & field)
{
  storeHelper(stream, field.dimensions(), nullptr);
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

  for (int n = 1; n <= nOldTimes; ++n)
  {
    writeField(stream, name, field.oldTime(n));
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

  auto && read_field = readField<T>(stream, foam_mesh);
  auto & field = foam_mesh.lookupObjectRef<T>(read_field.first);

  field == read_field.second;

  std::cout << "Deserialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL)
            << " " << field.name() << " timeindex: " << field.timeIndex() << std::endl;

  for (int nOld = 1; nOld <= nOldTimes; ++nOld)
  {
    auto old_read_field = readField<T>(stream, foam_mesh);
    field.oldTimeRef(nOld) == old_read_field.second;
    std::cout << "  - Deserialising " << old_read_field.first << ". nOld: " << nOld << std::endl;
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
    outputField(field.name() + "_out.txt", field, false);
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

// This functions nulls and clears old time information from the first step
// For geometric fields the underlying `base field' needs to be nulled for this to work.
// Unfortunately, OpenFOAM code is pretty opaque in terms of understanding this issue.
// constexpr ensures this is compiled statically as the base field is not present for
// dimensioned fields.
template <typename T>
void
removeOldTime(T & field)
{
  if constexpr (is_geometric_field<T>::value)
  {
    // Note that this does not work as intended and it seems that only the first
    // base field is removed.
    // TODO: fix to get Crank-Nicolson to work.
    if (field.nOldTimes() > 1)
      removeOldTime(field.oldTimeRef());

    // otbf is set in the setBase functions of the OldTimeField. This is mirrored here
    // in order to null it.
    auto & otbf = const_cast<typename T::Base::OldTime &>(Foam::OldTimeBaseFieldType<T>()(field));
    otbf.clearOldTimes();
    otbf.nullOldestTime();
  }
  field.clearOldTimes();
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
      removeOldTime(field);

      if (mesh.time().timeIndex() != field.timeIndex())
      {
        std::cout << "Field " << field.name() << " checked out." << std::endl;
        mesh.checkOut(field);
      }
    }

    // TODO: remove before merge
    outputField(field.name() + "_in.txt", field, false);
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

  storeFields<Foam::volScalarField, true>(stream, mesh, context);
  storeFields<Foam::volVectorField, true>(stream, mesh, context);
  storeFields<Foam::volTensorField, false>(stream, mesh, context);
  storeFields<Foam::volSymmTensorField, false>(stream, mesh, context);

  storeFields<Foam::surfaceScalarField, true>(stream, mesh, context);
  storeFields<Foam::surfaceVectorField, true>(stream, mesh, context);
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
