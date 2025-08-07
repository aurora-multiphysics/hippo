#include "FoamDataStore.h"

FoamDataStore::FoamDataStore(Foam::fvMesh & mesh) : _mesh(mesh) {}

void
FoamDataStore::storeOneScalarField(const Foam::volScalarField & field,
                                   std::vector<Foam::scalar>::iterator & it)
{
  auto const & values = field.internalField();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    *it = values[i];
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto const & patch_values = field.boundaryField()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      *it = patch_values[i];
      it++;
    }
  }
}

void
FoamDataStore::storeOneVectorField(const Foam::volVectorField & field,
                                   std::vector<Foam::Vector<Foam::scalar>>::iterator & it)
{
  auto const & values = field.internalField();
  for (auto i = 0; i < _mesh.nCells(); ++i)
  {
    *it = values[i];
    it++;
  }

  for (int patchID = 0; patchID < field.boundaryField().size(); ++patchID)
  {
    auto const & patch_values = field.boundaryField()[patchID];
    for (auto i = 0.; i < patch_values.size(); ++i)
    {
      *it = patch_values[i];
      it++;
    }
  }
}

void
FoamDataStore::storeFields()
{
  storeField(volScalarFields_, volScalarFieldsCopy_);
  storeField(volVectorFields_, volVectorFieldsCopy_);
  storeField(volTensorFields_, volTensorFieldsCopy_);
  storeField(volSymmTensorFields_, volSymmTensorFieldsCopy_);
}

template <typename T>
void
FoamDataStore::loadField(std::vector<T *> & field_vector, std::vector<T *> & field_vector_copy)
{
  for (uint i = 0; i < field_vector.size(); ++i)
  {
    auto & field_ = field_vector.at(i);
    auto & field_copy = field_vector_copy.at(i);
    assert(field_->name() == field_copy->name());

    *field_ == *field_copy;

    // Load the current time
    std::cout << "Loading field: " << field_->name() << std::endl;
    for (int n = 1; n <= field_->nOldTimes(); ++n)
    {
      std::cout << "\tLoading old time: " << n << std::endl;
      field_->oldTimeRef(n) == field_copy->oldTime(n);
    }
  }
}

template <typename T>
void
FoamDataStore::storeField(std::vector<T *> & field_vector, std::vector<T *> & field_vector_copy)
{

  for (auto & field : _mesh.curFields<T>())
  {
    if (std::find(field_vector.begin(), field_vector.end(), &field) == field_vector.end())
    {
      field_vector.push_back(&field);
      field_vector_copy.push_back(new T(field));
    }
  }
  for (int i = field_vector.size() - 1; i >= 0; --i)
  {
    if (!_mesh.foundObject<T>(field_vector.at(i)->name()))
    {
      field_vector.erase(field_vector.begin() + i);
      field_vector_copy.erase(field_vector_copy.begin() + i);
    }
  }

  for (uint i = 0; i < field_vector.size(); ++i)
  {
    *(field_vector_copy.at(i)) == (*field_vector.at(i));
  }
}

void
FoamDataStore::loadFields()
{

  std::cout << "Time: " << _mesh.time().userTimeValue()
            << ". Time index: " << _mesh.time().timeIndex() << std::endl;
  loadField(volScalarFields_, volScalarFieldsCopy_);
  loadField(volVectorFields_, volVectorFieldsCopy_);
  loadField(volTensorFields_, volTensorFieldsCopy_);
  loadField(volSymmTensorFields_, volSymmTensorFieldsCopy_);

  for (uint i = 0; i < volScalarFields_.size(); ++i)
  {
    checkField(*(volScalarFields_.at(i)), *(volScalarFieldsCopy_.at(i)));
  }
}

void
FoamDataStore::storeTime(Foam::Time & time)
{
  _cur_time.timeIndex = time.findClosestTimeIndex(time.times(), time.userTimeValue());
  _cur_time.deltaT = time.deltaTValue();
  _cur_time.time = time.userTimeValue();
  printf("Saved time: %d, %lf, %lf.\n", _cur_time.timeIndex, _cur_time.time, _cur_time.deltaT);
  fflush(stdout);
}

void
FoamDataStore::loadTime(Foam::Time & time)
{
  time.setTime(time, _cur_time.timeIndex);
  time.setDeltaTNoAdjust(_cur_time.deltaT);
  time.setTime(_cur_time.time, _cur_time.timeIndex);
  printf("Loaded time: %d, %lf, %lf.\n",
         time.findClosestTimeIndex(time.times(), time.userTimeValue()),
         time.userTimeValue(),
         time.userDeltaTValue());
  fflush(stdout);
}
