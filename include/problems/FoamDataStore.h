#pragma once

#include "DataIO.h"
#include "IOobject.H"
#include "IOstream.H"
#include "IStringStream.H"
#include "Istream.H"
#include "dictionary.H"
#include "fvMesh.H"
#include "volFields.H"
#include "volFieldsFwd.H"
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

inline void
dataStore(std::ostream & stream, FoamTimeState & s, void * context)
{
  storeHelper(stream, s.time, context);
  storeHelper(stream, s.deltaT, context);
  storeHelper(stream, s.timeIndex, context);
}

inline void
dataLoad(std::istream & stream, FoamTimeState & s, void * context)
{
  loadHelper(stream, s.time, context);
  loadHelper(stream, s.deltaT, context);
  loadHelper(stream, s.timeIndex, context);
}

template <typename T>
inline void
outputField(std::string fname, Foam::VolField<T> & field)
{
  Foam::OStringStream oss(Foam::IOstream::ASCII);
  oss << field;

  std::ofstream ofs{fname, std::ios::out | std::ios::trunc};
  ofs << field.name() << " " << field.time().userTimeValue() << std::endl;
  ofs << oss.str();

  for (int n = 1; n <= field.nOldTimes(); ++n)
  {
    Foam::OStringStream oss_old(Foam::IOstream::ASCII);
    oss_old << field.oldTimeRef(n);
    ofs << field.oldTimeRef(n).name() << " " << field.oldTimeRef(n).time().userTimeValue()
        << std::endl;
    ofs << oss_old.str();
  }
  ofs.close();
}

template <typename T>
void
dataStore(std::ostream & stream, std::vector<Foam::VolField<T> *> & field_vec, void * context)
{
  std::vector<std::pair<std::string, std::string>> data_vec;
  for (auto & field : field_vec)
  {
    Foam::OStringStream oss(Foam::IOstream::ASCII);
    oss << *field;
    data_vec.push_back(std::pair(field->name(), oss.str()));

    for (int n = 1; n <= field->nOldTimes(); ++n)
    {
      Foam::OStringStream oss_old(Foam::IOstream::ASCII);
      Foam::VolField<T> old_field{field->oldTime(n)};
      oss_old << old_field;
      data_vec.push_back(std::pair(old_field.name(), oss_old.str()));
    }
  }

  storeHelper(stream, data_vec, context);
}

template <typename T>
void
dataLoad(std::istream & stream, std::vector<Foam::VolField<T> *> & field_vec, void * context)
{
  std::vector<std::pair<std::string, std::string>> data_vec;
  loadHelper(stream, data_vec, context);
  std::vector<Foam::VolField<T> *> new_data;
  auto it = data_vec.begin();
  while (it != data_vec.end())
  {
    std::cout << "Deserialising " << it->first << std::endl;
    Foam::IStringStream iss{Foam::string(it->second), Foam::IOstream::ASCII};
    auto new_field = new Foam::VolField<T>{
        Foam::IOobject{it->first, field_vec[0]->mesh()},
        field_vec[0]->mesh(),
        Foam::dictionary(iss),
    };
    new_data.push_back(new_field);

    it++;
    int nOld = 0;
    while (it != data_vec.end() && it->first.size() > 2 &&
           it->first.substr(it->first.size() - 2, 2) == "_0")
    {
      Foam::IStringStream iss_0{Foam::string(it->second), Foam::IOstream::ASCII};
      new_field->oldTimeRef(++nOld) ==
          Foam::VolField<T>{Foam::IOobject{it->first, field_vec[0]->mesh()},
                            field_vec[0]->mesh(),
                            Foam::dictionary(iss_0)};
      std::cout << "-Deserialising " << it->first << ". nOld: " << nOld << std::endl;
      it++;
    }
  }
  field_vec = new_data;
}

inline void
dataStore(std::ostream & stream, FoamDataStore & s, void * context)
{
  printf("Data store being called!\n");
  fflush(stdout);
  storeHelper(stream, s._cur_time, context);

  // storeHelper(stream, s.volScalarFields_, context);
  dataStore(stream, s.volScalarFieldsCopy_, context);
  dataStore(stream, s.volVectorFieldsCopy_, context);
  dataStore(stream, s.volTensorFieldsCopy_, context);
  dataStore(stream, s.volSymmTensorFieldsCopy_, context);
}

inline void
dataLoad(std::istream & stream, FoamDataStore & s, void * context)
{
  printf("Data load being called!\n");
  fflush(stdout);

  loadHelper(stream, s._cur_time, context);
  s.loadTime(const_cast<Foam::Time &>(s._mesh.time()));

  dataLoad(stream, s.volScalarFieldsCopy_, context);
  dataLoad(stream, s.volVectorFieldsCopy_, context);
  dataLoad(stream, s.volTensorFieldsCopy_, context);
  dataLoad(stream, s.volSymmTensorFieldsCopy_, context);
}
