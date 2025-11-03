#include "FoamBCBase.h"
#include "FoamMassFlowRateMappedInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Pstream.H"
#include "PstreamReduceOps.H"
#include "Registry.h"

#include "ops.H"
#include "mpi.h"
#include "UPstream.H"
#include "vectorField.H"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamMassFlowRateMappedInletBC);

void
FoamMassFlowRateMappedInletBC::createPatchProcMap()
{
  // Use OpenFOAM's PStream to send local points to all processes
  // A little inefficient but this is only done on initialisation
  // A bounding box approach could be more efficient if there is any
  // changes that require repartitioning
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary = foam_mesh.boundary()[_boundary[0]];
  auto face_centres = boundary.Cf();

  Foam::PstreamBuffers send_points(Foam::UPstream::commsTypes::nonBlocking);
  for (int i = 0; i < Foam::Pstream::nProcs(); ++i)
  {
    Foam::UOPstream send(i, send_points);
    send << face_centres;
  }
  send_points.finishedSends(true);

  std::vector<MPI_Request> size_requests(Foam::Pstream::nProcs());
  std::vector<MPI_Request> data_requests(Foam::Pstream::nProcs());
  std::vector<int> sizes(Foam::Pstream::nProcs());
  for (int i = 0; i < Foam::Pstream::nProcs(); ++i)
  {
    // Recieve from each process and check required point is present
    Foam::vectorField field;
    Foam::UIPstream recieve(i, send_points);
    recieve >> field;
    auto & vec = _send_map[i];
    std::vector<int> recv_indices;
    for (int j = 0; j < field.size(); ++j)
    {
      auto index = foam_mesh.findCell(field[j] + _offset);
      if (index >= 0)
      {
        vec.push_back(index); // assign to send map required indices
        recv_indices.push_back(j);
      }
    }

    // Let original processes know which points will come from each rank
    auto size = static_cast<int>(vec.size());
    MPI_Isend(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &size_requests[i]);
    MPI_Isend(
        recv_indices.data(), recv_indices.size(), MPI_INT, i, 1, MPI_COMM_WORLD, &data_requests[i]);

    MPI_Irecv(&sizes[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &size_requests[i]);
  }

  MPI_Waitall(size_requests.size(), size_requests.data(), MPI_STATUSES_IGNORE);

  for (int i = 0; i < Foam::Pstream::nProcs(); ++i)
  {
    std::vector<int> recv_indices(sizes[i]);
    MPI_Recv(recv_indices.data(), sizes[i], MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (auto index : recv_indices)
      _recv_map[i].push_back(index);
  }
}

InputParameters
FoamMassFlowRateMappedInletBC::validParams()
{
  auto params = FoamBCBase::validParams();
  params.remove("v");
  params.remove("initial_condition");
  params.remove("foam_variable");

  params.addParam<std::string>(
      "foam_variable", "T", "Name of foam variable associated with velocity");

  params.addRequiredParam<std::vector<Real>>("translation_vector",
                                             "A vector indicating the location of recycling plane");
  params.addRequiredParam<PostprocessorName>("mass_flow_pp",
                                             "Postprocessors containing mass flow rate.");

  return params;
}

FoamMassFlowRateMappedInletBC::FoamMassFlowRateMappedInletBC(const InputParameters & params)
  : FoamBCBase(params),
    PostprocessorInterface(this),
    _pp_name(params.get<PostprocessorName>("mass_flow_pp")),
    _offset(),
    _send_map(),
    _recv_map()
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
FoamMassFlowRateMappedInletBC::getMappedArray(const Foam::word & name)
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

  Foam::PstreamBuffers sendBuf(Foam::UPstream::commsTypes::nonBlocking);
  auto & var = foam_mesh.lookupObject<Foam::VolField<T>>(name);
  for (int i = 0; i < Foam::Pstream::nProcs(); ++i)
  {
    Foam::Field<T> points(_send_map[i].size());
    for (auto j = 0lu; j < _send_map[i].size(); ++j)
      points[j] = var[_send_map[i][j]];

    Foam::UOPstream send(i, sendBuf);
    send << points;
  }
  sendBuf.finishedSends(true);

  Foam::Field<T> boundaryData(boundary_patch.size());
  for (int i = 0; i < Foam::Pstream::nProcs(); ++i)
  {
    Foam::UIPstream recv(i, sendBuf);
    Foam::Field<T> recvData;
    recv >> recvData;
    for (auto j = 0lu; j < _recv_map[i].size(); ++j)
    {
      boundaryData[_recv_map[i][j]] = recvData[j];
    }
  }

  return boundaryData;
}

void
FoamMassFlowRateMappedInletBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

  // should we mapping rho U or just U? Fo now U but we can change it
  auto pp_value = getPostprocessorValueByName(_pp_name);

  auto && U_map = getMappedArray<Foam::vector>("U");
  auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
  auto & Sf = boundary_patch.Sf();

  auto m_dot = Foam::sum(rho * (U_map & Sf));
  Foam::reduce(m_dot, Foam::sumOp<Real>());

  auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
      boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));

  U_var == -U_map * pp_value / m_dot;
}

void
FoamMassFlowRateMappedInletBC::initialSetup()
{
  _foam_variable = "";
}
