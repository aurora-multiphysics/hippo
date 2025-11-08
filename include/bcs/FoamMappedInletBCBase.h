#pragma once

#include "FoamPostprocessorBCBase.h"
#include "UPstream.H"

class FoamMappedInletBCBase : public FoamPostprocessorBCBase
{
public:
  static InputParameters validParams();

  FoamMappedInletBCBase(const InputParameters & params);

  virtual ~FoamMappedInletBCBase()
  {
    if (Foam::UPstream::parRun())
      Foam::UPstream::freeCommunicator(_foam_comm);
  }

protected:
  Foam::vector _offset;

  std::map<int, std::vector<int>> _send_map;

  std::map<int, std::vector<int>> _recv_map;

  Foam::label _foam_comm;

  MPI_Comm _mpi_comm;

  // create send and receive information for mapping
  void createPatchProcMap();

  // get array from mapped plane on the inlet processes
  template <typename T>
  Foam::Field<T> getMappedArray(const Foam::word & name);

  // check if bounding box intersects with rank
  bool intersectMapPlane(const Foam::fvMesh & mesh, Real cart_bbox[6]);

  // create/assign communicators for the transfers between map and inlet planes
  void createMapComm(const Foam::fvMesh & mesh,
                     Foam::vectorField face_centres,
                     std::vector<int> & send_process,
                     std::vector<int> & recv_process);
};
