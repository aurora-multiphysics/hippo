#include "FoamInterfaceImpl.h"
#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"
#include "libmesh/fe_type.h"
#include "functionObjects/field/wallHeatFlux/wallHeatFlux.H"
#include "objectRegistry.H"

registerMooseObject("hippoApp", FoamProblem);

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  params.addRequiredParam<std::string>(
      FoamProblem::OUTPUT_VARIABLE_NAME,
      "The name of the variable to write the OpenFOAM boundary temperature into.");
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
  auto params = FoamProblem::validParams();
  return params;
}

BuoyantFoamProblem::BuoyantFoamProblem(InputParameters const & params)
  : FoamProblem(params), _app(_interface)
{
}

void
BuoyantFoamProblem::addExternalVariables()
{
}

void
BuoyantFoamProblem::externalSolve()
{
  _app.run();
}

void
BuoyantFoamProblem::syncSolutions(Direction dir)
{
  auto & mesh = this->mesh();

  const auto out_var_id = parameters().get<std::string>(FoamProblem::OUTPUT_VARIABLE_NAME);
  auto & var = getVariable(0, out_var_id);
  _face_T = var.number();

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
    printf("subdomains.size(): %lu\n", subdomains.size());
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      auto n_added = _interface->getWallHeatFlux(foam_vol_t, subdomains[i]);
      printf("foam_vol_t=[");
      for (auto v : foam_vol_t)
      {
        printf("%f ", v);
      }
      printf("]\n");
      patch_counts[i] = n_added;
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
        auto dof = elem_ptr->dof_number(var.sys().number(), _face_T, 0);
        // var.sys().solution().set(dof, foam_vol_t[elem]);

        // TODO: does the above need to be negative, such that the flux is reversed? I.e.:
        var.sys().solution().set(dof, -foam_vol_t[elem]);
      }
    }
    var.sys().solution().close();
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    auto subdomains = mesh.getSubdomainList();
    // The number of elements in each subdomain of the mesh
    // Allocate an extra element as we'll accumulate these counts later
    std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      patch_counts[i] = _app.patch_size(subdomains[i]);
    }
    std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);

    // Retrieve the values from MOOSE
    std::vector<double> moose_T;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      std::vector<double> buf;
      // Set the face temperatures on the OpenFOAM mesh
      for (size_t elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
        assert(elem_ptr);
        // Find the dof number of the element
        auto dof = elem_ptr->dof_number(var.number(), _face_T, 0);

        // Insert the element's temperature into the MOOSE temperature vector
        var.sys().solution().get({dof}, buf);
        std::copy(buf.begin(), buf.end(), std::back_inserter(moose_T));
      }
      // Copy the values from the MOOSE temperature vector into OpenFOAM's
      _app.set_patch_face_t(subdomains[i], moose_T);
      moose_T.clear();
    }
    _interface->write();
  }
}

// Local Variables:
// mode: c++
// End:
