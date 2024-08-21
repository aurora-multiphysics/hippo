#include "FoamInterfaceImpl.h"
#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"
#include "MooseTypes.h"
#include "libmesh/fe_type.h"
#include "functionObjects/field/wallHeatFlux/wallHeatFlux.H"
#include "objectRegistry.H"

#include <algorithm>

registerMooseObject("hippoApp", FoamProblem);

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  params.addRequiredParam<std::string>(
      FoamProblem::WALL_TEMP_VAR, "The name of the variable to write the wall temperature into.");
  params.addRequiredParam<std::string>(
      FoamProblem::WALL_HEAT_FLUX_VAR,
      "The name of the variable to write the wall heat flux into.");
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

  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {
    const auto wall_heat_flux_var_id =
        parameters().get<std::string>(FoamProblem::WALL_HEAT_FLUX_VAR);
    auto & wall_heat_flux_var = getVariable(0, wall_heat_flux_var_id);
    _wall_heat_flux = wall_heat_flux_var.number();

    // Vector to hold the temperature on the elements in every subdomain
    // Not sure if we can pre-allocate this - we need the number of elements
    // in the subdomain owned by the current rank. We count this in a loop
    // later.
    std::vector<Real> foam_wall_heat_flux;

    auto subdomains = mesh.getSubdomainList();
    // The number of elements in each subdomain of the mesh
    // Allocate an extra element as we'll accumulate these counts later
    std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
    printf("subdomains.size(): %lu\n", subdomains.size());
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      auto n_added = _interface->getWallHeatFlux(foam_wall_heat_flux, subdomains[i]);
      patch_counts[i] = n_added;
    }
    std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      // Set the face temperatures on the MOOSE mesh
      printf("setting heat flux values on MOOSE mesh: [ ");
      for (auto elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
        assert(elem_ptr);
        auto dof = elem_ptr->dof_number(wall_heat_flux_var.sys().number(), _wall_heat_flux, 0);

        // Flip the heat flux since we want it as viewed from the other side of the boundary.
        auto hf_value = -foam_wall_heat_flux[elem];
        printf("%f ", hf_value);
        wall_heat_flux_var.sys().solution().set(dof, hf_value);
      }
      printf("]\n");
    }
    wall_heat_flux_var.sys().solution().close();
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    const auto wall_temperature_var_id = parameters().get<std::string>(FoamProblem::WALL_TEMP_VAR);
    auto & wall_temperature_var = getVariable(0, wall_temperature_var_id);
    _wall_temperature = wall_temperature_var.number();

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
        auto dof = elem_ptr->dof_number(wall_temperature_var.number(), _wall_temperature, 0);

        // Insert the element's temperature into the MOOSE temperature vector
        wall_temperature_var.sys().solution().get({dof}, buf);
        std::copy(buf.begin(), buf.end(), std::back_inserter(moose_T));
      }
      // Copy the values from the MOOSE temperature vector into OpenFOAM's
      printf("setting values on OpenFOAM patch: [ ");
      for (const auto v : moose_T)
      {
        printf("%f ", v);
      }
      printf("]\n");
      _app.set_patch_face_t(subdomains[i], moose_T);
      moose_T.clear();
    }
    _interface->write();
    wall_temperature_var.sys().solution().close();
  }
}

// Local Variables:
// mode: c++
// End:
