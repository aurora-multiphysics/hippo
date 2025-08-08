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
void
dataStore(std::ostream & stream, std::vector<Foam::VolField<T> *> & s, void * context)
{
  Foam::OStringStream oss(Foam::IOstream::ASCII);
  for (auto field : s)
    oss << *field;
  std::string data_str{oss.str()};

  storeHelper(stream, data_str, context);
}

template <typename T>
void
dataLoad(std::istream & stream, std::vector<Foam::VolField<T> *> & s, void * context)
{
  std::string data_str;
  loadHelper(stream, data_str, context);

  Foam::IStringStream iss(data_str, Foam::IOstream::ASCII);
  for (auto & field : s)
    iss >> *field;
}

inline void
dataStore(std::ostream & stream, FoamDataStore & s, void * context)
{
  printf("Data store being called!\n");
  fflush(stdout);
  storeHelper(stream, s._cur_time, context);
  std::vector<std::pair<std::string, std::string>> data_vec;
  for (auto & field : s.volScalarFieldsCopy_)
  {
    Foam::OStringStream oss(Foam::IOstream::ASCII);
    oss << *field;
    data_vec.push_back(std::pair(field->name(), oss.str()));
  }

  storeHelper(stream, data_vec, context);
  std::ofstream ofs("geometric_fields_in.txt", std::ios::out | std::ios::trunc);
  for (auto & data : data_vec)
  {
    ofs << data.first << std::endl;
    ofs << data.second << std::endl;
  }
  ofs.close();
  // storeHelper(stream, s.volScalarFields_, context);
  // storeHelper(stream, s.volScalarFieldsCopy_, context);
  // storeHelper(stream, s.volVectorFields_, context);
  // storeHelper(stream, s.volVectorFieldsCopy_, context);
  // storeHelper(stream, s.volTensorFields_, context);
  // storeHelper(stream, s.volTensorFieldsCopy_, context);
  // storeHelper(stream, s.volSymmTensorFields_, context);
  // storeHelper(stream, s.volSymmTensorFieldsCopy_, context);
  // storeHelper(stream, s._cur_time, context);
}

inline void
dataLoad(std::istream & stream, FoamDataStore & s, void * context)
{
  printf("Data load being called!\n");
  fflush(stdout);

  loadHelper(stream, s._cur_time, context);
  std::vector<std::pair<std::string, std::string>> data_vec;
  loadHelper(stream, data_vec, context);
  std::vector<Foam::volScalarField *> new_data;
  for (auto & data : data_vec)
  {
    Foam::IStringStream iss{Foam::string(data.second), Foam::IOstream::ASCII};
    new_data.push_back(new Foam::volScalarField{
        Foam::IOobject{data.first, s._mesh},
        s._mesh,
        Foam::dictionary(iss),
    });
  }
  // assert(s.volScalarFieldsCopy_ == new_data);
  s.volScalarFieldsCopy_ = std::move(new_data);

  std::vector<std::pair<std::string, std::string>> data_vec_;
  for (auto & field : s.volScalarFieldsCopy_)
  {
    Foam::OStringStream oss(Foam::IOstream::ASCII);
    oss << *field;
    data_vec_.push_back(std::pair(field->name(), oss.str()));
  }
  std::ofstream ofs("geometric_fields_out.txt", std::ios::out | std::ios::trunc);
  for (auto & data : data_vec_)
  {
    ofs << data.first << std::endl;
    ofs << data.second << std::endl;
  }
  ofs.close();

  // loadHelper(stream, s.volScalarFields_, context);
  // loadHelper(stream, s.volScalarFieldsCopy_, context);
  // loadHelper(stream, s.volVectorFields_, context);
  // loadHelper(stream, s.volVectorFieldsCopy_, context);
  // loadHelper(stream, s.volTensorFields_, context);
  // loadHelper(stream, s.volTensorFieldsCopy_, context);
  // loadHelper(stream, s.volSymmTensorFields_, context);
  // loadHelper(stream, s.volSymmTensorFieldsCopy_, context);
  // loadHelper(stream, s._cur_time, context);
}
