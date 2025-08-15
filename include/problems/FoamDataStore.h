#pragma once

#include "DataIO.h"

#include "fvCFD_moose.h"

#include <fstream>
#include <ostream>
#include <cxxabi.h>

template <typename T>
inline void
dataStore(std::ostream & stream, Foam::Vector<T> & s, void * context)
{
  storeHelper(stream, s.x(), context);
  storeHelper(stream, s.y(), context);
  storeHelper(stream, s.z(), context);
}

template <typename T>
inline void
dataLoad(std::ostream & stream, Foam::Vector<T> & s, void * context)
{
  loadHelper(stream, s.x(), context);
  loadHelper(stream, s.y(), context);
  loadHelper(stream, s.z(), context);
}

template <typename T>
inline void
outputField(std::string fname, const T & field)
{
  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss << field;

  std::ofstream ofs{fname, std::ios::out | std::ios::trunc};
  ofs << field.name() << " " << field.time().userTimeValue() << std::endl;
  ofs << oss.str();

  for (int n = 1; n <= field.nOldTimes(); ++n)
  {
    Foam::OStringStream oss_old(Foam::IOstream::ASCII);
    oss_old << field.oldTime(n);
    ofs << field.oldTime(n).name() << " " << field.oldTime(n).time().userTimeValue() << std::endl;
    ofs << oss_old.str();
  }
  ofs.close();
}

template <typename T>
inline void
dataStoreField(std::ostream & stream, const T & field, void * context)
{
  auto nOldTimes{field.nOldTimes()};
  storeHelper(stream, nOldTimes, context);
  std::cout << "Serialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL) << " "
            << field.name() << std::endl;

  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss.precision(17);
  oss << field;
  auto store_pair{std::pair(std::string(field.name()), std::string(oss.str()))};
  storeHelper(stream, store_pair, context);

  for (int n = 1; n <= field.nOldTimes(); ++n)
  {
    Foam::OStringStream oss_old(Foam::IOstream::ASCII);
    oss_old.precision(17);
    T old_field{field.oldTime(n)};
    oss_old << old_field;
    auto old_store_pair = std::pair(std::string(old_field.name()), std::string(oss_old.str()));
    storeHelper(stream, old_store_pair, context);
    std::cout << "  - Serialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL)
              << " " << old_field.name() << std::endl;
  }
}

template <typename T>
inline void
dataLoadField(std::istream & stream, Foam::fvMesh & foam_mesh)
{

  Foam::label nOldTimes;
  loadHelper(stream, nOldTimes, nullptr);

  std::pair<std::string, std::string> field_data;
  loadHelper(stream, field_data, nullptr);
  auto & field = foam_mesh.lookupObjectRef<T>(field_data.first);

  std::cout << "Deserialising " << abi::__cxa_demangle(typeid(field).name(), NULL, NULL, NULL)
            << " " << field_data.first << std::endl;

  Foam::IStringStream iss{Foam::string(field_data.second), Foam::IOstream::ASCII};
  field == T{
               Foam::IOobject{field_data.first,
                              foam_mesh,
                              Foam::IOobject::NO_READ,
                              Foam::IOobject::NO_WRITE,
                              false},
               foam_mesh,
               Foam::dictionary(iss),
           };

  for (int nOld = 1; nOld <= nOldTimes; ++nOld)
  {
    std::pair<std::string, std::string> old_field_data;
    loadHelper(stream, old_field_data, nullptr);
    Foam::IStringStream iss_0{Foam::string(old_field_data.second), Foam::IOstream::ASCII};
    T old_field{
        Foam::IOobject{
            field_data.first, foam_mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE, false},
        foam_mesh,
        Foam::dictionary(iss_0),
    };
    field.oldTimeRef(nOld) == old_field;
    std::cout << "  - Deserialising " << old_field_data.first << ". nOld: " << nOld << std::endl;
  }
}

template <typename T, bool strict>
inline void
storeFields(std::ostream & stream, const Foam::fvMesh & mesh, void * context)
{
  auto && cur_fields{mesh.curFields<T>()};
  auto nFields{0};
  for (auto & field : cur_fields)
  {
    if ((Foam::isType<T>(field) && strict) || (Foam::isA<T>(field) && !strict))
      ++nFields;
  }

  storeHelper(stream, nFields, context);
  for (T & field : cur_fields)
  {
    if ((Foam::isType<T>(field) && strict) || (Foam::isA<T>(field) && !strict))
    {
      outputField<T>(field.name() + "_out.txt", field);
      dataStoreField<T>(stream, field, context);
    }
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
  for (auto & field : mesh.fields<T>(true))
  {
    if (!field.isOldTime())
      outputField<T>(field.name() + "_in.txt", field);
  }
}

template <>
inline void
dataStore(std::ostream & stream, const Foam::Time & time, void * context)
{
  auto timeIndex = time.findClosestTimeIndex(time.times(), time.userTimeValue());
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

  time.setTime(time, timeIndex);
  time.setDeltaTNoAdjust(deltaT);
  time.setTime(timeValue, timeIndex);
  printf("Loaded time: %d, %lf, %lf.\n",
         time.findClosestTimeIndex(time.times(), time.userTimeValue()),
         time.userTimeValue(),
         time.userDeltaTValue());
  fflush(stdout);
}

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

  storeFields<Foam::volScalarField::Internal, true>(stream, mesh, context);
}

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

  loadFields<Foam::volScalarField::Internal>(stream, mesh, context);
}
