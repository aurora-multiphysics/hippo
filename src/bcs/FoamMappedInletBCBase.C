#include "FoamMappedInletBCBase.h"

#include <InputParameters.h>
#include <MooseTypes.h>

#include <mpi.h>
#include <UPstream.H>
#include <pointInCell.H>
#include <vectorField.H>
#include <volFieldsFwd.H>
#include <Pstream.H>

#include <algorithm>
#include <cassert>
#include <limits>
#include <sstream>
#include <vector>

namespace
{
// Get the cartesian bounding box of the mapped inlet plane
void
getBBox(const Foam::vectorField & points, Real bbox[6], MPI_Comm comm)
{
  bbox[0] = std::numeric_limits<Real>::max();
  bbox[1] = std::numeric_limits<Real>::lowest();
  bbox[2] = std::numeric_limits<Real>::max();
  bbox[3] = std::numeric_limits<Real>::lowest();
  bbox[4] = std::numeric_limits<Real>::max();
  bbox[5] = std::numeric_limits<Real>::lowest();
  for (auto p : points)
  {
    bbox[0] = std::min(bbox[0], p.x());
    bbox[1] = std::max(bbox[1], p.x());
    bbox[2] = std::min(bbox[2], p.y());
    bbox[3] = std::max(bbox[3], p.y());
    bbox[4] = std::min(bbox[4], p.z());
    bbox[5] = std::max(bbox[5], p.z());
  }

  MPI_Allreduce(MPI_IN_PLACE, &bbox[0], 1, MPI_DOUBLE, MPI_MIN, comm);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[1], 1, MPI_DOUBLE, MPI_MAX, comm);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[2], 1, MPI_DOUBLE, MPI_MIN, comm);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[3], 1, MPI_DOUBLE, MPI_MAX, comm);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[4], 1, MPI_DOUBLE, MPI_MIN, comm);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[5], 1, MPI_DOUBLE, MPI_MAX, comm);
}

enum class LocatorIssue
{
  NOT_FOUND,
  MANY_FOUND
};

std::vector<int>
findLocatorIssues(const std::vector<int> & indices, MPI_Comm comm, LocatorIssue issue)
{
  std::vector<int> found(indices.size(), 0);
  std::vector<int> gl_found(indices.size(), 0);

  for (auto i = 0lu; i < indices.size(); ++i)
  {
    found[i] = indices[i] >= 0;
  }
  MPI_Allreduce(found.data(), gl_found.data(), found.size(), MPI_INT, MPI_SUM, comm);

  std::vector<int> conflict_indices;
  for (auto i = 0lu; i < indices.size(); ++i)
  {
    if (issue == LocatorIssue::MANY_FOUND && gl_found[i] > 1)
      conflict_indices.push_back(i);
    else if (issue == LocatorIssue::NOT_FOUND && gl_found[i] == 0)
      conflict_indices.push_back(i);
  }

  return conflict_indices;
}

void
retainClosestCells(std::vector<int> & indices,
                   const std::vector<int> & conflict_indices,
                   const Foam::pointField & locations,
                   const Foam::vector & offset,
                   const Foam::vectorField & cell_centres,
                   MPI_Comm comm)
{
  std::vector<Real> local_distances(conflict_indices.size(), std::numeric_limits<Real>::max());
  std::vector<Real> minimum_distances(conflict_indices.size());

  for (auto i = 0lu; i < conflict_indices.size(); ++i)
  {
    const int idx = conflict_indices[i];
    if (indices[idx] >= 0)
      local_distances[i] = Foam::mag(locations[idx] + offset - cell_centres[indices[idx]]);
  }

  MPI_Allreduce(local_distances.data(),
                minimum_distances.data(),
                conflict_indices.size(),
                MPI_DOUBLE,
                MPI_MIN,
                comm);

  for (auto i = 0lu; i < conflict_indices.size(); ++i)
    if (local_distances[i] != minimum_distances[i])
      indices[conflict_indices[i]] = -1;
}

std::string
getLocationList(const Foam::pointField & locations, const std::vector<int> & indices)
{
  std::stringstream msg;
  for (auto idx : indices)
  {
    const auto & loc = locations[idx];
    msg << "\t(" << loc.x() << ", " << loc.y() << ", " << loc.z() << ")\n";
  }
  return msg.str();
}

void
retainLowestRank(std::vector<int> & indices,
                 const std::vector<int> & conflict_indices,
                 MPI_Comm comm)
{
  int rank;
  MPI_Comm_rank(comm, &rank);

  std::vector<int> minimum_ranks(conflict_indices.size(), std::numeric_limits<int>::max());
  for (auto i = 0lu; i < conflict_indices.size(); ++i)
    if (indices[conflict_indices[i]] >= 0)
      minimum_ranks[i] = rank;

  MPI_Allreduce(
      MPI_IN_PLACE, minimum_ranks.data(), conflict_indices.size(), MPI_INT, MPI_MIN, comm);

  for (auto i = 0lu; i < conflict_indices.size(); ++i)
    if (rank != minimum_ranks[i])
      indices[conflict_indices[i]] = -1;
}
}

bool
FoamMappedInletBCBase::intersectMapPlane(const Foam::fvMesh & mesh, Real cart_bbox[6])
{
  auto & vertices = mesh.points();
  for (int i = 0; i < mesh.nCells(); ++i)
  {
    auto points = mesh.cellPoints(i);
    Real cell_bbox[6] = {std::numeric_limits<Real>::max(),
                         std::numeric_limits<Real>::lowest(),
                         std::numeric_limits<Real>::max(),
                         std::numeric_limits<Real>::lowest(),
                         std::numeric_limits<Real>::max(),
                         std::numeric_limits<Real>::lowest()};
    for (auto point : points)
    {
      const Foam::point & p = vertices[point];
      cell_bbox[0] = std::min(cell_bbox[0], p.x());
      cell_bbox[1] = std::max(cell_bbox[1], p.x());
      cell_bbox[2] = std::min(cell_bbox[2], p.y());
      cell_bbox[3] = std::max(cell_bbox[3], p.y());
      cell_bbox[4] = std::min(cell_bbox[4], p.z());
      cell_bbox[5] = std::max(cell_bbox[5], p.z());
    }
    if (cart_bbox[0] <= cell_bbox[1] && cart_bbox[1] >= cell_bbox[0] &&
        cart_bbox[2] <= cell_bbox[3] && cart_bbox[3] >= cell_bbox[2] &&
        cart_bbox[4] <= cell_bbox[5] && cart_bbox[5] >= cell_bbox[4])
    {
      return true;
    }
  }

  return false;
}

void
FoamMappedInletBCBase::createMapComm(const Foam::fvMesh & mesh,
                                     const Foam::vectorField & face_centres,
                                     std::vector<int> & map_processes,
                                     std::vector<int> & inlet_processes,
                                     MPI_Comm comm)
{
  Real cart_bbox[6];
  auto mapped_plane = face_centres + _offset;
  getBBox(mapped_plane(), cart_bbox, comm);

  int mappedPlaneProcess = intersectMapPlane(mesh, cart_bbox);
  int inletPlaneProcess = !face_centres.empty();

  std::vector<int> inlet_procs(Foam::UPstream::nProcs());
  std::vector<int> map_procs(Foam::UPstream::nProcs());

  MPI_Allgather(&mappedPlaneProcess, 1, MPI_INT, map_procs.data(), 1, MPI_INT, comm);
  MPI_Allgather(&inletPlaneProcess, 1, MPI_INT, inlet_procs.data(), 1, MPI_INT, comm);

  map_processes.clear();
  inlet_processes.clear();

  // create list of processes in new communicator and whether they are in the inlet or mapped plane
  std::vector<int> processes;
  int j = 0;
  for (int i = 0; i < Foam::UPstream::nProcs(); ++i)
  {
    if (inlet_procs[i] || map_procs[i])
    {
      processes.push_back(i);
      if (inlet_procs[i])
        inlet_processes.push_back(j);
      if (map_procs[i])
        map_processes.push_back(j);
      ++j;
    }
  }

  _foam_comm = createCommunicator(Foam::UPstream::worldComm, processes, _mpi_comm);
}

void
FoamMappedInletBCBase::createPatchProcMap()
{
  auto & boundary = getFvMesh().boundary()[_boundary[0]];
  auto face_centres = boundary.Cf();

  std::vector<int> map_procs, inlet_procs;
  const MPI_Comm comm{Foam::PstreamGlobals::MPICommunicators_[Foam::UPstream::worldComm]};
  createMapComm(getFvMesh(), face_centres, map_procs, inlet_procs, comm);

  if (map_procs.empty())
    mooseError("The mapped inlet plane does not intersect the mesh.");

  if (_mpi_comm == MPI_COMM_NULL) // process not in mapped or inlet planes
    return;

  Foam::PstreamBuffers send_points(
      Foam::UPstream::commsTypes::nonBlocking, Foam::UPstream::msgType(), _foam_comm);
  int rank = Foam::UPstream::myProcNo(_foam_comm);
  bool isMapProc = std::find(map_procs.begin(), map_procs.end(), rank) != map_procs.end();
  bool isInletProc = std::find(inlet_procs.begin(), inlet_procs.end(), rank) != inlet_procs.end();

  if (isInletProc) // send points from inlet process to all map processes
  {
    for (int proc : map_procs)
    {
      Foam::UOPstream send(proc, send_points);
      send << face_centres;
    }
  }

  send_points.finishedSends(true);

  std::vector<int> send_sizes(inlet_procs.size());
  std::vector<MPI_Request> size_requests(inlet_procs.size());
  std::vector<MPI_Request> data_requests(inlet_procs.size());
  std::vector<std::vector<int>> recv_indices_procs(inlet_procs.size());
  MPI_Comm map_comm;
  auto foam_map_comm = createCommunicator(_foam_comm, map_procs, map_comm);

  if (isMapProc) // check points from each process to see if they are local
  {
    for (auto i = 0lu; i < inlet_procs.size(); ++i)
    {
      Foam::vectorField field;
      Foam::UIPstream receive(inlet_procs[i], send_points);
      receive >> field;
      auto & vec = _send_map[inlet_procs[i]];
      auto & recv_indices = recv_indices_procs[i];
      auto indices = findIndices(field + _offset, map_comm);

      for (auto j = 0lu; j < indices.size(); ++j)
      {
        if (indices[j] >= 0)
        {
          vec.push_back(indices[j]); // assign to send map required indices
          recv_indices.push_back(j);
        }
      }
      if (vec.empty())
        _send_map.erase(inlet_procs[i]);

      // Let original processes know which points will come from each rank
      send_sizes[i] = recv_indices.size();
      MPI_Isend(&send_sizes[i], 1, MPI_INT, inlet_procs[i], 0, _mpi_comm, &size_requests.at(i));
      MPI_Isend(recv_indices.data(),
                recv_indices.size(),
                MPI_INT,
                inlet_procs[i],
                1,
                _mpi_comm,
                &data_requests.at(i));
    }
  }

  destroyCommunicator(foam_map_comm);

  if (isInletProc) // create map to determine where data from map processes should go
  {
    std::vector<unsigned int> assignment_count(face_centres.size(), 0);
    for (const auto & proc : map_procs)
    {
      int size{0};
      MPI_Recv(&size, 1, MPI_INT, proc, 0, _mpi_comm, MPI_STATUS_IGNORE);

      std::vector<int> recv_indices(size);
      MPI_Recv(recv_indices.data(), size, MPI_INT, proc, 1, _mpi_comm, MPI_STATUS_IGNORE);
      for (const auto & index : recv_indices)
      {
        if (index < 0 || index >= face_centres.size())
          mooseError(
              "Received invalid mapped inlet face index ", index, " from process ", proc, ".");

        _recv_map[proc].push_back(index);
        ++assignment_count[index];
      }
    }

    for (auto i = 0lu; i < assignment_count.size(); ++i)
      if (assignment_count[i] != 1)
        mooseError("Mapped inlet face at (",
                   face_centres[i].x(),
                   ", ",
                   face_centres[i].y(),
                   ", ",
                   face_centres[i].z(),
                   ") was assigned to ",
                   assignment_count[i],
                   " mapped cells; expected exactly one.");
  }

  if (isMapProc)
  {
    MPI_Waitall(inlet_procs.size(), size_requests.data(), MPI_STATUSES_IGNORE);
    MPI_Waitall(inlet_procs.size(), data_requests.data(), MPI_STATUSES_IGNORE);
  }
}

std::vector<int>
FoamMappedInletBCBase::findIndices(const Foam::pointField & locations, MPI_Comm comm)
{
  std::vector<int> indices(locations.size(), -1);
  for (auto i = 0; i < locations.size(); ++i)
  {
    indices[i] = _mesh_searcher.findCell(locations[i], Foam::pointInCellShapes::facePlanes);
  }

  auto missing_indices = findLocatorIssues(indices, comm, LocatorIssue::NOT_FOUND);
  for (auto i = 0lu; i < missing_indices.size(); ++i)
  {
    int celli = _mesh_searcher.findNearestCell(locations[missing_indices[i]]);
    if (celli < 0)
      continue;

    bool in_cell = Foam::pointInCell(
        locations[missing_indices[i]], getFvMesh(), celli, Foam::pointInCellShapes::facePlanes);
    indices[missing_indices[i]] = (in_cell) ? celli : -1;
  }

  // Check whether any locations are not found
  std::vector<int> notfound_indices = findLocatorIssues(indices, comm, LocatorIssue::NOT_FOUND);
  if (!notfound_indices.empty())
  {
    auto err_msg = getLocationList(locations, notfound_indices);
    mooseError("Locations not found:\n", err_msg);
  }

  // Resolve locations claimed by multiple ranks
  auto conflict_indices = findLocatorIssues(indices, comm, LocatorIssue::MANY_FOUND);
  if (conflict_indices.empty())
    return indices;

  // Prefer the cell whose centre is closest to the mapped location.
  retainClosestCells(
      indices, conflict_indices, locations, Foam::vector::zero, getFvMesh().cellCentres(), comm);

  // Check again for conflicts: distance criterion could leave ties
  conflict_indices = findLocatorIssues(indices, comm, LocatorIssue::MANY_FOUND);
  if (conflict_indices.empty())
    return indices;

  // Break remaining ties using the distance to the original inlet location.
  retainClosestCells(
      indices, conflict_indices, locations, -_offset, getFvMesh().cellCentres(), comm);

  conflict_indices = findLocatorIssues(indices, comm, LocatorIssue::MANY_FOUND);
  if (!conflict_indices.empty())
  {
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (rank == 0)
      mooseWarning("Mapped locations assigned to multiple ranks after deconflicting. Using the "
                   "lowest rank:\n",
                   getLocationList(locations, conflict_indices));

    retainLowestRank(indices, conflict_indices, comm);
  }

  return indices;
}

InputParameters
FoamMappedInletBCBase::validParams()
{
  auto params = FoamPostprocessorBCBase::validParams();
  params.addRequiredParam<std::vector<Real>>("translation_vector",
                                             "A vector indicating the location of recycling plane");

  return params;
}

FoamMappedInletBCBase::FoamMappedInletBCBase(const InputParameters & params)
  : FoamPostprocessorBCBase(params, FoamBCType::fixedValue),
    _offset(),
    _send_map(),
    _recv_map(),
    _foam_comm(0),
    _mpi_comm(MPI_COMM_NULL),
    _mesh_searcher(Foam::meshSearch::New(getFvMesh()))
{
  if (_boundary.size() > 1)
    mooseError("There can only be one boundary using this method");

  auto param_offset = params.get<std::vector<Real>>("translation_vector");
  assert(param_offset.size() == 3);

  _offset = {param_offset[0], param_offset[1], param_offset[2]};
  createPatchProcMap();
}

template <typename T>
Foam::Field<T>
FoamMappedInletBCBase::getMappedArray(const Foam::word & name)
{
  if (_mpi_comm == MPI_COMM_NULL)
    return Foam::Field<T>();

  auto & boundary_patch = getFvMesh().boundary()[_boundary[0]];

  Foam::PstreamBuffers sendBuf(
      Foam::UPstream::commsTypes::nonBlocking, Foam::UPstream::msgType(), _foam_comm);
  if (!_send_map.empty())
  {
    auto & var = getFvMesh().lookupObject<Foam::VolField<T>>(name);
    for (const auto & pair : _send_map)
    {
      auto & proc = pair.first;
      auto & send_indices = pair.second;

      Foam::Field<T> points(send_indices.size());
      for (auto j = 0lu; j < send_indices.size(); ++j)
        points[j] = var[send_indices[j]];

      Foam::UOPstream send(proc, sendBuf);
      send << points;
    }
  }
  sendBuf.finishedSends(true);

  Foam::Field<T> boundaryData(boundary_patch.size());
  if (!_recv_map.empty())
  {
    for (const auto & pair : _recv_map)
    {
      auto & proc = pair.first;
      auto & recv_indices = pair.second;

      Foam::UIPstream recv(proc, sendBuf);
      Foam::Field<T> recvData;
      recv >> recvData;
      for (auto j = 0lu; j < recv_indices.size(); ++j)
      {
        boundaryData[recv_indices[j]] = recvData[j];
      }
    }
  }

  return boundaryData;
}

Foam::label
FoamMappedInletBCBase::createCommunicator(const Foam::label parent_comm,
                                          const std::vector<int> & procs,
                                          MPI_Comm & new_comm)
{
  Foam::label foam_comm;
  if (Foam::UPstream::parRun())
  {
    Foam::labelList foam_procs(procs.begin(), procs.end());
    foam_comm = Foam::UPstream::allocateCommunicator(parent_comm, foam_procs, true);
    new_comm = Foam::PstreamGlobals::MPICommunicators_[foam_comm];
  }
  else
  {
    foam_comm = Foam::UPstream::worldComm;
    new_comm = Foam::PstreamGlobals::MPICommunicators_[Foam::UPstream::worldComm];
  }
  return foam_comm;
}

void
FoamMappedInletBCBase::destroyCommunicator(Foam::label comm)
{
  if (Foam::UPstream::parRun())
    Foam::UPstream::freeCommunicator(comm);
}
