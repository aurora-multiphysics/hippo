#pragma once

#include "DataIO.h"
#include "fvMesh.H"
#include "volFields.H"
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

// inline void
// dataStore(std::ostream & stream, FoamDataStore & s, void * context)
// {
//   storeHelper(stream, s._scalar_map, context);
//   storeHelper(stream, s._vector_map, context);
//   storeHelper(stream, s._cur_time, context);
// }

// inline void
// dataLoad(std::istream & stream, FoamDataStore & s, void * context)
// {
//   loadHelper(stream, s._scalar_map, context);
//   loadHelper(stream, s._vector_map, context);
//   loadHelper(stream, s._cur_time, context);
// }
