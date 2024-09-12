#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/enum_order.h"
#include "libmesh/fe_type.h"

registerMooseObject("hippoApp", FoamProblem);

namespace
{
constexpr auto PARAM_VAR_FOAM_HF = "foam_heat_flux";
constexpr auto PARAM_VAR_FOAM_T = "foam_temp";
constexpr auto PARAM_VAR_T = "temp";

bool
is_constant_monomial(const MooseVariableFieldBase & var)
{
  return var.order() == libMesh::Order::CONSTANT && var.feType().family == FEFamily::MONOMIAL;
}
}

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();

  // Parameters to set variables to read from/write to.
  // Note that these can all point to the same variable to save memory.
  params.addParam<std::string>(
      PARAM_VAR_FOAM_HF, "The name of the aux variable to write the OpenFOAM wall heat flux into.");
  params.addParam<std::string>(
      PARAM_VAR_FOAM_T,
      "The name of the aux variable to write the OpenFOAM boundary temperature into.");
  params.addParam<std::string>(
      PARAM_VAR_T, "The name of the aux variable to read the boundary temperature from.");

  return params;
}

FoamProblem::FoamProblem(InputParameters const & params)
  : ExternalProblem(params),
    _foam_mesh(dynamic_cast<FoamMesh *>(&this->ExternalProblem::mesh())),
    _interface(_foam_mesh->getFoamInterface())
{
  assert(_foam_mesh);
  assert(_interface);

  auto t_var_name = params.get<std::string>(PARAM_VAR_T);
  if (t_var_name.empty())
  {
    mooseError("Parameter '", PARAM_VAR_T, "' must be set.");
  }

  auto foam_t_var_name = params.get<std::string>(PARAM_VAR_FOAM_T);
  auto foam_hf_var_name = params.get<std::string>(PARAM_VAR_FOAM_HF);
  if (foam_t_var_name.empty() && foam_hf_var_name.empty())
  {
    mooseError("At least one of the following parameters must be set: '",
               PARAM_VAR_FOAM_T,
               "', '",
               PARAM_VAR_FOAM_HF,
               "'.");
  }
  if (foam_t_var_name == foam_hf_var_name)
  {
    mooseError("Parameters '",
               PARAM_VAR_FOAM_T,
               "' and '",
               PARAM_VAR_FOAM_HF,
               "' cannot refer to the same variable: '",
               foam_t_var_name,
               "'.");
  }
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
    auto foam_t_var_name = parameters().get<std::string>(PARAM_VAR_FOAM_T);
    auto & foam_t_var = getVariable(0, foam_t_var_name);
    auto foam_hf_var_name = parameters().get<std::string>(PARAM_VAR_FOAM_HF);
    auto & foam_hf_var = getVariable(0, foam_hf_var_name);
    if (!is_constant_monomial(foam_t_var) && !is_constant_monomial(foam_hf_var))
    {
      mooseError("variables to store temperature or heat flux must have:\n"
                 "  family = MONOMIAL\n"
                 "  order = CONSTANT\n");
    }

    // Vector to hold the temperature on the elements in every subdomain
    // Not sure if we can pre-allocate this - we need the number of elements
    // in the subdomain owned by the current rank. We count this in a loop
    // later.
    std::vector<Real> foam_t;
    std::vector<Real> foam_hf;

    auto subdomains = mesh.getSubdomainList();
    // The number of elements in each subdomain of the mesh
    // Allocate an extra element as we'll accumulate these counts later
    std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      auto n_added_t = _app.append_patch_face_T(subdomains[i], foam_t);
      auto n_added_hf = _interface->getWallHeatFlux(foam_hf, subdomains[i]);
      assert(n_added_t == n_added_hf);

      patch_counts[i] = n_added_t;
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

        auto dof_t = elem_ptr->dof_number(foam_t_var.sys().number(), foam_t_var.number(), 0);
        foam_t_var.sys().solution().set(dof_t, foam_t[elem]);

        // Take negative of heat flux to change direction.
        auto dof_hf = elem_ptr->dof_number(foam_hf_var.sys().number(), foam_hf_var.number(), 0);
        foam_hf_var.sys().solution().set(dof_hf, -foam_hf[elem]);
      }
    }
    foam_t_var.sys().solution().close();
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    auto t_var_name = parameters().get<std::string>(PARAM_VAR_T);
    auto & t_var = getVariable(0, t_var_name);
    if (!is_constant_monomial(t_var))
    {
      mooseError("variables to store temperature or heat flux must have:\n"
                 "  family = MONOMIAL\n"
                 "  order = CONSTANT\n");
    }

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
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (auto i = 0U; i < subdomains.size(); ++i)
    {
      std::vector<double> moose_T;
      // Set the face temperatures on the OpenFOAM mesh
      for (size_t elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
      {
        auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
        assert(elem_ptr);
        // Find the dof number of the element
        auto dof = elem_ptr->dof_number(t_var.sys().number(), t_var.number(), 0);

        // Insert the element's temperature into the MOOSE temperature vector
        auto t_value = t_var.sys().solution()(dof);
        moose_T.emplace_back(t_value);
      }
      // Copy the values from the MOOSE temperature vector into OpenFOAM's
      _app.set_patch_face_t(subdomains[i], moose_T);
    }
    _interface->write();
  }
}

// Local Variables:
// mode: c++
// End:
