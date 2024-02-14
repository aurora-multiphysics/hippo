#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"

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
  auto serial = mesh.isSerial();

  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {

    std::vector<Real> foamT;
    foamT.reserve(mesh.nNodes());

    std::vector<Real> foam_vol_t;
    // foamVolT.reserve(mesh.nCells());

    auto subdomains = mesh.getSubdomainList();
    // First we get all the temperature data for every subdomain
    std::vector<size_t> patch_counts_vol(subdomains.size() + 1, 0);
    {
      int i = 0;
      for (auto const & subdom : subdomains)
      {
        patch_counts_vol[i++] = _app.append_patch_face_T(subdom, foam_vol_t);
      }
    }
    std::exclusive_scan(
        patch_counts_vol.begin(), patch_counts_vol.end(), patch_counts_vol.begin(), 0);

    // Find the rank offsets into the MOOSE mesh's element array
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    auto n_elems = patch_counts_vol.back();
    auto num_communicators = _aux->comm().size();
    std::vector<std::size_t> rank_offsets(num_communicators, 0);
    MPI_Allgather(&n_elems, 1, MPIU_SIZE_T, rank_offsets.data(), 1, MPIU_SIZE_T, MPI_COMM_WORLD);
    std::exclusive_scan(rank_offsets.begin(), rank_offsets.end(), rank_offsets.begin(), 0);

    for (int i = 0; i < subdomains.size(); ++i)
    {
      // Set the face temperatures on the MOOSE mesh
      for (int elem = patch_counts_vol[i]; elem < patch_counts_vol[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + rank_offsets.at(rank));
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
