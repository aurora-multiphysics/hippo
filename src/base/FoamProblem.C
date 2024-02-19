#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"

namespace
{
/// Find the offset into a patch's element array for each rank.
/// Gather the number of elements each rank is responsible for, and accumulate
/// them to get the offset for each rank into the overall element array.
std::vector<size_t>
find_mpi_rank_offsets(const size_t n_elems, const MPI_Comm communicator)
{
  int comm_size;
  MPI_Comm_size(communicator, &comm_size);
  std::vector<std::size_t> rank_offsets(comm_size, 0);
  MPI_Allgather(&n_elems, 1, MPIU_SIZE_T, rank_offsets.data(), 1, MPIU_SIZE_T, communicator);
  std::exclusive_scan(rank_offsets.begin(), rank_offsets.end(), rank_offsets.begin(), 0);
  return rank_offsets;
}
}

registerMooseObject("hippoApp", FoamProblem);

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  // std::vector<std::string> empty_vec;
  // params.addParam<std::vector<std::string>>(
  //    "foam_args", empty_vec, "List of arguments to be passed to openFoam solver");
  // TODO: needs to be a vector
  return params;
}

FoamProblem::FoamProblem(InputParameters const & params)
  : ExternalProblem(params),
    _foam_mesh(dynamic_cast<FoamMesh *>(&this->ExternalProblem::mesh())),
    _interface(_foam_mesh->getFoamInterface())
{
  assert(_foam_mesh);
  assert(_interface);
}

void
FoamProblem::externalSolve()
{
}

registerMooseObject("hippoApp", BuoyantFoamProblem);

InputParameters
BuoyantFoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  return params;
}

BuoyantFoamProblem::BuoyantFoamProblem(InputParameters const & params)
  : FoamProblem(params), _app(_interface)
// TODO: Assuming the temp var is "T" should pass the name in
{
}

void
BuoyantFoamProblem::addExternalVariables()
{
  InputParameters params = _factory.getValidParams("MooseVariable");
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
  addAuxVariable("MooseVariable", "foamT_face", params);
  _face_T = _aux->getFieldVariable<Real>(0, "foamT_face").number();
}

void
BuoyantFoamProblem::externalSolve()
{
  _app.run();
}

void
BuoyantFoamProblem::syncSolutions(Direction dir)
{
  auto & mesh = static_cast<FoamMesh &>(this->mesh());

  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {
    // Vector to hold the temperature on the elements in every subdomain
    // Not sure if we can pre-allocate this - we need the number of elements
    // in the subdomain owned by the current rank. We count this in a loop
    // later.
    std::vector<Real> foam_vol_t;

    auto subdomains = mesh.getSubdomainList();
    // The number of elements in each subdomain of the mesh
    // Allocate an extra element as we'll accumulate these counts later
    std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      patch_counts[i] = _app.append_patch_face_T(subdomains[i], foam_vol_t);
    }
    std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);

    // The offsets for each rank into the MOOSE mesh's elements
    auto rank_offsets = find_mpi_rank_offsets(patch_counts.back(), MPI_COMM_WORLD);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (int i = 0; i < subdomains.size(); ++i)
    {
      // Set the face temperatures on the MOOSE mesh
      for (int elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + rank_offsets[rank]);
        assert(elem_ptr);
        auto dof = elem_ptr->dof_number(_aux->number(), _face_T, 0);
        _aux->solution().set(dof, foam_vol_t[elem]);
      }
    }
    _aux->solution().close();
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    ;
  }
}

// Local Variables:
// mode: c++
// End:
