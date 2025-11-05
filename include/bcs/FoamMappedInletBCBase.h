#pragma once

#include "FoamBCBase.h"
#include "UPstream.H"

class FoamMappedInletBCBase : public FoamBCBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  FoamMappedInletBCBase(const InputParameters & params);

  virtual void initialSetup() override;

  virtual ~FoamMappedInletBCBase()
  {
    if (Foam::UPstream::parRun())
      Foam::UPstream::freeCommunicator(_foam_comm);
  }

protected:
  PostprocessorName _pp_name;

  Foam::vector _offset;

  std::map<int, std::vector<int>> _send_map;

  std::map<int, std::vector<int>> _recv_map;

  Foam::label _foam_comm;

  MPI_Comm _mpi_comm;

  void createPatchProcMap();

  template <typename T>
  Foam::Field<T> getMappedArray(const Foam::word & name);

  bool intersectMapPlane(const Foam::fvMesh & mesh, Real cart_bbox[6]);

  void createMapComm(const Foam::fvMesh & mesh,
                     Foam::vectorField face_centres,
                     std::vector<int> & send_process,
                     std::vector<int> & recv_process);
};
