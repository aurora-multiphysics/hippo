#pragma once

#include "DataIO.h"

#include "IOobject.H"
#include "fvCFD_moose.h"
#include "tmp.H"

#include <fstream>
#include <ostream>
#include <vector>

struct FoamTimeState
{
  Foam::label timeIndex;
  Foam::scalar time;
  Foam::scalar deltaT;
};

class FoamDataStore
{
public:
  FoamDataStore() = delete;
  FoamDataStore(Foam::fvMesh & mesh);
  void storeFields();
  void loadFields();
  void storeTime(Foam::Time & time);
  void loadTime(Foam::Time & time);

  bool checkField(const Foam::volScalarField & field1, const Foam::volScalarField & field2)
  {
    std::vector<Foam::scalar> copy_field1(_get_field_size());
    std::vector<Foam::scalar> copy_field2(_get_field_size());
    auto it1 = copy_field1.begin();
    storeOneScalarField(field1, it1);
    auto it2 = copy_field2.begin();
    storeOneScalarField(field2, it2);

    return copy_field1 == copy_field2;
  }

private:
  Foam::fvMesh & _mesh;

  inline int64_t _get_field_size() const
  {
    int64_t size = _mesh.nCells();
    for (const auto & patch : _mesh.boundary())
    {
      size += patch.size();
    }
    return size;
  }
  inline int64_t _get_buffer_size() const;
  void storeOneScalarField(const Foam::volScalarField & field,
                           std::vector<Foam::scalar>::iterator & it);
  void storeOneVectorField(const Foam::volVectorField & field,
                           std::vector<Foam::Vector<Foam::scalar>>::iterator & it);
  template <typename T>
  void loadField(std::vector<T *> & field_vector, std::vector<T *> & field_vector_copy);
  template <typename T>
  void storeField(std::vector<T *> & field_vector, std::vector<T *> & field_vector_copy);

  FoamTimeState _cur_time;
  std::vector<Foam::volScalarField *> volScalarFields_;
  std::vector<Foam::volVectorField *> volVectorFields_;
  std::vector<Foam::volTensorField *> volTensorFields_;
  std::vector<Foam::volSymmTensorField *> volSymmTensorFields_;
  std::vector<Foam::volScalarField *> volScalarFieldsCopy_;
  std::vector<Foam::volVectorField *> volVectorFieldsCopy_;
  std::vector<Foam::volTensorField *> volTensorFieldsCopy_;
  std::vector<Foam::volSymmTensorField *> volSymmTensorFieldsCopy_;

  friend inline void dataStore(std::ostream & stream, FoamDataStore & s, void * context);
  friend inline void dataLoad(std::istream & stream, FoamDataStore & s, void * context);
};

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

template <template <typename> class FieldType, typename T>
inline void
outputField(std::string fname, const FieldType<T> & field)
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

template <template <typename> class FieldType, typename T>
inline void
dataStore(std::ostream & stream, const FieldType<T> & field, void * context)
{
  auto nOldTimes{field.nOldTimes()};
  storeHelper(stream, nOldTimes, context);

  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss << field;
  auto store_pair{std::pair(std::string(field.name()), std::string(oss.str()))};
  storeHelper(stream, store_pair, context);

  for (int n = 1; n <= field.nOldTimes(); ++n)
  {
    Foam::OStringStream oss_old(Foam::IOstream::ASCII);
    FieldType<T> old_field{field.oldTime(n)};
    oss_old << old_field;
    auto old_store_pair = std::pair(std::string(old_field.name()), std::string(oss_old.str()));
    storeHelper(stream, old_store_pair, context);
  }
}

template <template <typename> class FieldType, typename T>
inline void
dataLoad(std::istream & stream, Foam::fvMesh & foam_mesh)
{

  Foam::label nOldTimes;
  loadHelper(stream, nOldTimes, nullptr);

  std::pair<std::string, std::string> field_data;
  loadHelper(stream, field_data, nullptr);
  auto & field = foam_mesh.lookupObjectRef<FieldType<T>>(field_data.first);

  std::cout << "Deserialising " << field_data.first << std::endl;
  Foam::IStringStream iss{Foam::string(field_data.second), Foam::IOstream::ASCII};
  field == FieldType<T>{
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
    FieldType<T> old_field{
        Foam::IOobject{
            field_data.first, foam_mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE, false},
        foam_mesh,
        Foam::dictionary(iss_0),
    };
    field.oldTimeRef(nOld) == old_field;
    std::cout << "-Deserialising " << old_field_data.first << ". nOld: " << nOld << std::endl;
    outputField<FieldType, T>(field.name() + "_in.txt", field);
  }
}

template <template <typename> class FieldType, typename T>
inline void
storeFields(std::ostream & stream, const Foam::fvMesh & mesh, void * context)
{
  auto && cur_fields{mesh.curFields<FieldType<T>>()};
  auto nFields{cur_fields.size()};

  storeHelper(stream, nFields, context);
  for (FieldType<T> & field : cur_fields)
  {
    outputField<FieldType, T>(field.name() + "_out.txt", field);
    dataStore<FieldType, T>(stream, field, context);
  }
}

template <template <typename> class FieldType, typename T>
inline void
loadFields(std::istream & stream, Foam::fvMesh & mesh, void * context)
{
  int nFields{};
  loadHelper(stream, nFields, context);
  for (int i = 0; i < nFields; ++i)
  {
    dataLoad<FieldType, T>(stream, mesh);
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

  storeFields<Foam::VolField, Foam::scalar>(stream, mesh, context);
  storeFields<Foam::VolField, Foam::vector>(stream, mesh, context);
  storeFields<Foam::VolField, Foam::tensor>(stream, mesh, context);
  storeFields<Foam::VolField, Foam::symmTensor>(stream, mesh, context);

  storeFields<Foam::SurfaceField, Foam::scalar>(stream, mesh, context);
  storeFields<Foam::SurfaceField, Foam::vector>(stream, mesh, context);
  storeFields<Foam::SurfaceField, Foam::tensor>(stream, mesh, context);
  storeFields<Foam::SurfaceField, Foam::symmTensor>(stream, mesh, context);
}

template <>
inline void
dataLoad(std::istream & stream, Foam::fvMesh & mesh, void * context)
{
  loadHelper(stream, const_cast<Foam::Time &>(mesh.time()), context);

  loadFields<Foam::VolField, Foam::scalar>(stream, mesh, context);
  loadFields<Foam::VolField, Foam::vector>(stream, mesh, context);
  loadFields<Foam::VolField, Foam::tensor>(stream, mesh, context);
  loadFields<Foam::VolField, Foam::symmTensor>(stream, mesh, context);

  loadFields<Foam::SurfaceField, Foam::scalar>(stream, mesh, context);
  loadFields<Foam::SurfaceField, Foam::vector>(stream, mesh, context);
  loadFields<Foam::SurfaceField, Foam::tensor>(stream, mesh, context);
  loadFields<Foam::SurfaceField, Foam::symmTensor>(stream, mesh, context);
}
