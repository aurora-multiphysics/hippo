#pragma once

#include "HippoPtr.h"

#include <mpi.h>

#include <cstdint>
#include <memory>
#include <vector>
/*
 * Some utility template functions that are useful for gathering mesh
 * info to construct a Moose mesh
 * Not really optimised - e.g. in pracitice the count and offset
 * arrays should be be re-used
 */

namespace Hippo
{
/* Hack to get the MPI_Datatype from a c++ type at compile time - seems over the top */
template <typename T>
inline MPI_Datatype
get_mpi_type()
{
  if constexpr (std::is_same<T, int32_t>::value)
    return MPI_INT32_T;
  if constexpr (std::is_same<T, int64_t>::value)
    return MPI_INT64_T;
  if constexpr (std::is_same<T, uint32_t>::value)
    return MPI_UINT32_T;
  if constexpr (std::is_same<T, uint64_t>::value)
    return MPI_UINT64_T;
  if constexpr (std::is_same<T, double>::value)
    return MPI_DOUBLE;
  if constexpr (std::is_same<T, float>::value)
    return MPI_FLOAT;
  assert(false && "Unsupported data type");
  return MPI_BYTE;
}

/* All gathers vec into a HippoPtr (unique_ptr + size) on every rank */
template <typename T>
HippoPtr<T>
gather_vector(std::vector<T> & vec, MPI_Comm & comm, int32_t over_allocate = 0)
{
  int mpi_size, mpi_rank;
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);

  int32_t vec_size = vec.size() * sizeof(T);
  std::unique_ptr<int32_t[]> count(new int32_t[mpi_size + 1]);
  MPI_Allgather(&vec_size, 1, MPI_INT, count.get(), 1, MPI_INT, comm);
  std::unique_ptr<int32_t[]> offset(new int32_t[mpi_size + 1]);
  std::exclusive_scan(count.get(), count.get() + mpi_size + 1, offset.get(), 0);
  HippoPtr<T> glob_vec(offset[mpi_size] / sizeof(T) + over_allocate);

  MPI_Allgatherv(vec.data(),
                 count[mpi_rank],
                 MPI_BYTE,
                 glob_vec.begin(),
                 count.get(),
                 offset.get(),
                 MPI_BYTE,
                 comm);
  return glob_vec;
}

/* As above but does an exclusive scan on the vectors too */
template <typename T>
HippoPtr<T>
gather_and_scan_vector(std::vector<T> & vec, MPI_Comm & comm)
{
  int mpi_size, mpi_rank;
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);

  T local_total = std::accumulate(vec.begin(), vec.end(), 0);
  T scan = 0;
  auto global_total = local_total;
  MPI_Request req[2];
  MPI_Iexscan(&local_total, &scan, 1, get_mpi_type<T>(), MPI_SUM, comm, req);
  MPI_Iallreduce(MPI_IN_PLACE, &global_total, 1, get_mpi_type<T>(), MPI_SUM, comm, req + 1);
  MPI_Status stat[2];
  MPI_Waitall(2, req, stat);
  if (mpi_rank == 0)
    scan = 0;

  std::exclusive_scan(vec.begin(), vec.end(), vec.begin(), scan);

  auto result = gather_vector<T>(vec, comm, 1);
  result[result.size() - 1] = global_total;
  return result;
}

template <typename T>
T
mpi_scan(T & value, MPI_Comm & comm)
{
  T out;
  MPI_Exscan(&value, &out, 1, get_mpi_type<T>(), MPI_SUM, comm);
  int mpi_rank;
  MPI_Comm_rank(comm, &mpi_rank);
  if (mpi_rank == 0)
  {
    return 0;
  }
  return out;
}

}
// Local Variables:
// mode: c++
// End:
