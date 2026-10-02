#pragma once

#include "FoamPostprocessorBCBase.h"
#include <UPstream.H>
#include <pointFieldsFwd.H>
#include <vector>

class FoamMappedInletBCBase : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamMappedInletBCBase(const InputParameters & params);

  ~FoamMappedInletBCBase() override { destroyCommunicator(_foam_comm); }

protected:
  Foam::vector _offset;

  std::map<int, std::vector<int>> _send_map;

  std::map<int, std::vector<int>> _recv_map;

  Foam::label _foam_comm;

  MPI_Comm _mpi_comm;

  const Foam::meshSearch & _mesh_searcher;

  // get array from mapped plane on the inlet processes
  template <typename T>
  Foam::Field<T> getMappedArray(const Foam::word & name);

private:
  // create send and receive information for mapping
  void createPatchProcMap();

  // check if bounding box intersects with rank
  bool intersectMapPlane(const Foam::fvMesh & mesh, Real cart_bbox[6]);

  // create/assign communicators for the transfers between map and inlet planes
  void createMapComm(const Foam::fvMesh & mesh,
                     const Foam::vectorField & face_centres,
                     std::vector<int> & map_processes,
                     std::vector<int> & inlet_processes,
                     MPI_Comm comm);

  // find indices of cells containing mapped inlet points or raise error if not found
  std::vector<int> findIndices(const Foam::pointField & locations, MPI_Comm comm);

  // handle creation of new communicators in parallel or serial
  Foam::label createCommunicator(const Foam::label parent_comm,
                                 const std::vector<int> & procs,
                                 MPI_Comm & new_comm);

  // free communicators if parallel run
  void destroyCommunicator(Foam::label comm);
};
