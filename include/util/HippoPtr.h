#pragma once
#include <memory>
#include <vector>
#include <numeric>
#include <cassert>
/*
   A convenince structure to allocate a fixed array
   not using a vector mostly because I want to use this with
   mpi and vectors will be annoying as have to be initialised
*/
namespace Hippo
{
template <typename T>
class HippoPtr
{
  std::unique_ptr<T[]> _ptr = nullptr;
  size_t _size = 0;

public:
  HippoPtr(size_t new_size) : _ptr(new T[new_size]), _size(new_size) {}
  HippoPtr(std::vector<T> const & vec) : HippoPtr(vec.size())
  {
    std::copy(vec.begin(), vec.end(), _ptr.get());
  }
  HippoPtr() = default;

  size_t size() const { return _size; }

  T & operator[](size_t i)
  {
    assert(i < _size && i >= 0);
    return _ptr[i];
  }

  T const & operator[](size_t i) const
  {
    assert(i < _size && i >= 0);
    return _ptr[i];
  }

  T back() const { return _ptr[_size - 1]; }

  T * begin() { return _ptr.get(); }
  T * end() { return _ptr.get() + _size; }
  T const * begin() const { return _ptr.get(); }
  T const * end() const { return _ptr.get() + _size; }

  std::vector<T> to_std_vector() const { return std::vector<T>(begin(), end()); }
};

template <typename T>
HippoPtr<T>
scan_vec_to_pointer(std::vector<T> const & vec)
{
  auto result = HippoPtr<T>(vec.size() + 1);
  result[0] = 0;
  // we want an exclusive scan really with the total in .ptr[vec.size()]
  // but this will get the same result
  // and saves having to modify the vector or any other nonsense
  std::inclusive_scan(vec.begin(), vec.end(), result.begin() + 1);
  return result;
}
template <typename T>
HippoPtr<T>
copy_vec_to_pointer(std::vector<T> const & vec)
{
  return HippoPtr<T>(vec);
}

}
// Local Variables:
// mode: c++
// End:
