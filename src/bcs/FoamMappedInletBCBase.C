#include "FoamMappedInletBCBase.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "Pstream.H"

#include "mpi.h"
#include "UPstream.H"
#include "petsclog.h"
#include "vectorField.H"
#include "volFieldsFwd.H"
#include <algorithm>
#include <cassert>
#include <cfloat>

namespace
{
// Get the cartesian bounding box of the mapped inlet plane
void
getBBox(const Foam::vectorField points, Real bbox[6])
{
  bbox[0] = DBL_MAX;
  bbox[1] = DBL_MIN;
  bbox[2] = DBL_MAX;
  bbox[3] = DBL_MIN;
  bbox[4] = DBL_MAX;
  bbox[5] = DBL_MIN;
  for (auto p : points)
  {
    bbox[0] = std::min(bbox[0], p.x());
    bbox[1] = std::max(bbox[1], p.x());
    bbox[2] = std::min(bbox[2], p.y());
    bbox[3] = std::max(bbox[3], p.y());
    bbox[4] = std::min(bbox[4], p.z());
    bbox[5] = std::max(bbox[5], p.z());
  }

  MPI_Allreduce(MPI_IN_PLACE, &bbox[0], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[1], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[2], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[3], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[4], 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
  MPI_Allreduce(MPI_IN_PLACE, &bbox[5], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
}
}

bool
FoamMappedInletBCBase::intersectMapPlane(const Foam::fvMesh & mesh, Real cart_bbox[6])
{
  auto & vertices = mesh.points();
  for (int i = 0; i < mesh.nCells(); ++i)
  {
    auto points = mesh.cellPoints(i);
    for (auto pointI : points)
    {
      // check whether cell intersects bbox
      const Foam::point & p = vertices[pointI];
      if (p.x() >= cart_bbox[0] && p.x() <= cart_bbox[1] && p.y() >= cart_bbox[2] &&
          p.y() <= cart_bbox[3] && p.z() >= cart_bbox[4] && p.z() <= cart_bbox[5])
      {
        return true;
      }
    }

    // The bbox could by narrower than the cell, check in each direction whether bbox is within the
    // cell bbox
    Real cell_bbox[6] = {DBL_MAX, DBL_MIN, DBL_MAX, DBL_MIN, DBL_MAX, DBL_MIN};
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
    // check is cart bbox is narrower than cell bbox in x direction
    if ((cart_bbox[0] >= cell_bbox[0] && cart_bbox[1] <= cell_bbox[1]) &&
        (cart_bbox[3] >= cell_bbox[2] || cart_bbox[2] <= cell_bbox[3]) &&
        (cart_bbox[5] >= cell_bbox[4] || cart_bbox[4] <= cell_bbox[5]))
    {
      return true;
    }

    // check is cart bbox is narrower than cell bbox in y direction
    if ((cart_bbox[2] >= cell_bbox[2] && cart_bbox[3] <= cell_bbox[3]) &&
        (cart_bbox[1] >= cell_bbox[0] || cart_bbox[0] <= cell_bbox[1]) &&
        (cart_bbox[5] >= cell_bbox[4] || cart_bbox[4] <= cell_bbox[5]))
    {
      return true;
    }
    // check is cart bbox is narrower than cell bbox in z direction
    if ((cart_bbox[4] >= cell_bbox[4] && cart_bbox[5] <= cell_bbox[5]) &&
        (cart_bbox[1] >= cell_bbox[0] || cart_bbox[0] <= cell_bbox[1]) &&
        (cart_bbox[3] >= cell_bbox[2] || cart_bbox[2] <= cell_bbox[3]))
    {
      return true;
    }
  }

  return false;
}

void
FoamMappedInletBCBase::createMapComm(const Foam::fvMesh & mesh,
                                     const Foam::vectorField face_centres,
                                     std::vector<int> & map_process,
                                     std::vector<int> & inlet_process)
{
  Real cart_bbox[6];
  auto mapped_plane = face_centres + _offset;
  getBBox(mapped_plane(), cart_bbox);

  int mappedPlaneProcess = intersectMapPlane(mesh, cart_bbox);
  int inletPlaneProcess = face_centres.size() > 0;

  std::vector<int> inlet_procs(Foam::UPstream::nProcs());
  std::vector<int> map_procs(Foam::UPstream::nProcs());

  MPI_Allgather(&mappedPlaneProcess, 1, MPI_INT, map_procs.data(), 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Allgather(&inletPlaneProcess, 1, MPI_INT, inlet_procs.data(), 1, MPI_INT, MPI_COMM_WORLD);

  map_process.clear();
  inlet_process.clear();

  // create list of processes in new communicator and whether they are in the inlet or mapped plane
  std::vector<int> processes;
  int j = 0;
  for (int i = 0; i < Foam::UPstream::nProcs(); ++i)
  {
    if (inlet_procs[i] || map_procs[i])
    {
      processes.push_back(i);
      if (inlet_procs[i])
        inlet_process.push_back(j);
      if (map_procs[i])
        map_process.push_back(j);
      ++j;
    }
  }

  _foam_comm = createCommunicator(Foam::UPstream::worldComm, processes, _mpi_comm);
}

void
FoamMappedInletBCBase::createPatchProcMap()
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary = foam_mesh.boundary()[_boundary[0]];
  auto face_centres = boundary.Cf();

  std::vector<int> map_procs, inlet_procs;
  createMapComm(foam_mesh, face_centres, map_procs, inlet_procs);

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
      Foam::UIPstream recieve(inlet_procs[i], send_points);
      recieve >> field;
      auto & vec = _send_map[inlet_procs[i]];
      auto & recv_indices = recv_indices_procs[i];
      for (int j = 0; j < field.size(); ++j)
      {
        auto index = findIndex(field[j] + _offset, map_comm);
        if (index >= 0)
        {
          vec.push_back(index); // assign to send map required indices
          recv_indices.push_back(j);
        }
      }
      if (vec.size() == 0)
        _send_map.erase(inlet_procs[i]);

      // Let original processes know which points will come from each rank
      int size = recv_indices.size();
      MPI_Isend(&size, 1, MPI_INT, inlet_procs[i], 0, _mpi_comm, &size_requests.at(i));
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
    for (auto & proc : map_procs)
    {
      int size;
      MPI_Recv(&size, 1, MPI_INT, proc, 0, _mpi_comm, MPI_STATUS_IGNORE);

      std::vector<int> recv_indices(size);
      MPI_Recv(recv_indices.data(), size, MPI_INT, proc, 1, _mpi_comm, MPI_STATUS_IGNORE);
      for (auto & index : recv_indices)
      {
        assert(index < face_centres.size());
        _recv_map[proc].push_back(index);
      }
    }
  }
  MPI_Barrier(_mpi_comm);
}

int
FoamMappedInletBCBase::findIndex(const Foam::point & location, const MPI_Comm & comm)
{
  /*
  This function uses several ways of finding the mapped plane cell
  1. use findCell function
    - Sometimes on cell boundaries it may not find the cell or two processes with both find it
  2. If none found, find the closest point and do an expanded bounding box search, raise error
     if none still found.
  3. If multiple found, use the cell with cell centre closer to desired location
  4. If still multiple found, use the cell closer to the inlet.
    - In the unlikely chance there are still multiple cells detected, raise a warning
  Note that it is possible there is some non-deterministic behaviour in the function but this
  shouldn't be a problem in practice.
  */
  int index = _mesh->fvMesh().findCell(location, Foam::polyMesh::FACE_PLANES);
  int gl_index;
  MPI_Allreduce(&index, &gl_index, 1, MPI_INT, MPI_MAX, comm);

  // expand cell bounding box and repeat search
  if (gl_index < 0)
  {
    Foam::label celli = _mesh->fvMesh().findNearestCell(location);

    bool in_cell = _mesh->fvMesh().pointInCellBB(location, celli, 0.1);
    index = (in_cell) ? celli : -1;
    MPI_Allreduce(&index, &gl_index, 1, MPI_INT, MPI_MAX, comm);

    int rank;
    MPI_Comm_rank(comm, &rank);
    if (gl_index < 0 && rank == 0)
    {
      mooseError("Face centre at location (",
                 location[0],
                 ",",
                 location[1],
                 ",",
                 location[2],
                 ") does not have a mapped plane location");
    }
  }

  // use cell with cell centre closest to the location
  Foam::scalar dist{DBL_MAX}, gl_dist;
  if (index != -1)
    dist = Foam::mag(_mesh->fvMesh().cellCentres()[index] - location);

  MPI_Allreduce(&dist, &gl_dist, 1, MPI_DOUBLE, MPI_MIN, comm);
  if (dist != gl_dist)
    index = -1;

  // 2. use cell centre closest to inlet point
  int in_cell = index != -1;
  MPI_Allreduce(MPI_IN_PLACE, &in_cell, 1, MPI_INT, MPI_SUM, comm);
  if (in_cell > 1)
  {
    if (index != -1)
      dist = Foam::mag(_mesh->fvMesh().cellCentres()[index] - (location - _offset));
    MPI_Allreduce(&dist, &gl_dist, 1, MPI_DOUBLE, MPI_MIN, comm);
    if (dist != gl_dist)
      index = -1;
  }
  in_cell = index != -1;
  MPI_Allreduce(MPI_IN_PLACE, &in_cell, 1, MPI_INT, MPI_SUM, comm);

  if (in_cell > 1)
    mooseWarning("More than 1 process found location (",
                 location[0],
                 ",",
                 location[1],
                 ",",
                 location[2],
                 ")");
  return index;
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
  : FoamPostprocessorBCBase(params),
    _offset(),
    _send_map(),
    _recv_map(),
    _foam_comm(0),
    _mpi_comm(MPI_COMM_NULL)
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

  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

  Foam::PstreamBuffers sendBuf(
      Foam::UPstream::commsTypes::nonBlocking, Foam::UPstream::msgType(), _foam_comm);
  if (_send_map.size() > 0)
  {
    auto & var = foam_mesh.lookupObject<Foam::VolField<T>>(name);
    for (auto & pair : _send_map)
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
  if (_recv_map.size() > 0)
  {
    for (auto & pair : _recv_map)
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
