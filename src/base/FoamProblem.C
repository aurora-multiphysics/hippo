#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"

registerMooseObject("hippoApp", FoamProblem);

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
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

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      // Set the face temperatures on the MOOSE mesh
      for (auto elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
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
