#include "ExternalProblem.h"
#include "FoamMesh.h"
#include "FoamProblem.h"
#include "FoamSolver.h"
#include "VariadicTable.h"
#include "word.H"

#include <AuxiliarySystem.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <algorithm>
#include "FoamVariableField.h"
#include "InputParameters.h"
#include "VariadicTable.h"
#include <finiteVolume/solver/solver.H>
#include <fvMesh.H>
#include <libmesh/enum_order.h>
#include <libmesh/fe_type.h>
#include <string>

registerMooseObject("hippoApp", FoamProblem);

namespace
{
constexpr auto PARAM_VAR_FOAM_HF = "foam_heat_flux";
constexpr auto PARAM_VAR_FOAM_T = "foam_temp";
constexpr auto PARAM_VAR_T = "temp";
constexpr auto PARAM_VAR_HF = "heat_flux";

bool
is_constant_monomial(const MooseVariableFieldBase & var)
{
  return var.order() == libMesh::Order::CONSTANT && var.feType().family == FEFamily::MONOMIAL;
}

Real
variableValueAtElement(const libMesh::Elem * element, MooseVariableFieldBase * variable)
{
  auto & sys = variable->sys();
  auto dof = element->dof_number(sys.number(), variable->number(), 0);
  return sys.solution()(dof);
}
} // namespace

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();

  // Parameters to set variables to read from/write to.
  // Note that these can be omitted or point to the same variable to save
  // memory.
  params.addParam<std::string>(PARAM_VAR_FOAM_HF,
                               "The name of the aux variable to write the "
                               "OpenFOAM wall heat flux into.");
  params.addParam<std::string>(PARAM_VAR_FOAM_T,
                               "The name of the aux variable to write the "
                               "OpenFOAM boundary temperature into.");
  params.addParam<std::string>(
      PARAM_VAR_HF, "The name of the aux variable to set the OpenFOAM wall heat flux from.");
  params.addParam<std::string>(PARAM_VAR_T,
                               "The name of the aux variable to set the "
                               "OpenFOAM boundary temperature from.");
  return params;
}

FoamProblem::FoamProblem(InputParameters const & params)
  : ExternalProblem(params),
    _foam_mesh(dynamic_cast<FoamMesh *>(&this->ExternalProblem::mesh())),
    _solver(Foam::solver::New(_foam_mesh->fvMesh().time().controlDict().lookupOrDefault<Foam::word>(
                                  "solver", "fluid"),
                              _foam_mesh->fvMesh())
                .ptr()),
    _foam_variables(),
    _foam_bcs()
{
  assert(_foam_mesh);

  auto t_var_name = params.get<std::string>(PARAM_VAR_T);
  auto hf_var_name = params.get<std::string>(PARAM_VAR_HF);

  if (t_var_name == hf_var_name && !t_var_name.empty())
  {
    mooseError("Parameters '",
               PARAM_VAR_T,
               "' and '",
               PARAM_VAR_HF,
               "' cannot refer to the same variable: '",
               t_var_name,
               "'.");
  }

  auto foam_t_var_name = params.get<std::string>(PARAM_VAR_FOAM_T);
  auto foam_hf_var_name = params.get<std::string>(PARAM_VAR_FOAM_HF);

  if (foam_t_var_name == foam_hf_var_name && !foam_t_var_name.empty())
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
FoamProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  // Get FoamVariables create by the action AddFoamVariableAction
  TheWarehouse::Query query_vars = theWarehouse().query().condition<AttribSystem>("FoamVariable");
  query_vars.queryInto(_foam_variables);

  verifyFoamVariables();

  // Get FoamBCs create by the action AddFoamBCAction
  TheWarehouse::Query query_bcs = theWarehouse().query().condition<AttribSystem>("FoamBC");
  query_bcs.queryInto(_foam_bcs);

  verifyFoamBCs();
}

void
FoamProblem::externalSolve()
{
  if (parameters().get<bool>("solve"))
  {
    _solver.setTimeDelta(_dt); // Needed for constant deltaT cases
    _solver.run();
  }
}

void
FoamProblem::syncSolutions(Direction dir)
{
  if (!parameters().get<bool>("solve"))
  {
    return;
  }
  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {
    auto transfer_wall_temp = !parameters().get<std::string>(PARAM_VAR_FOAM_T).empty();
    auto transfer_wall_heat_flux = !parameters().get<std::string>(PARAM_VAR_FOAM_HF).empty();
    if (transfer_wall_temp)
    {
      if (transfer_wall_heat_flux)
      {
        syncFromOpenFoam<SyncVariables::Both>();
      }
      else
      {
        syncFromOpenFoam<SyncVariables::WallTemperature>();
      }
    }
    else if (transfer_wall_heat_flux)
    {
      syncFromOpenFoam<SyncVariables::WallHeatFlux>();
    }

    // Loop of shadowed variables and perform transfer
    for (auto & var : _foam_variables)
    {
      var->transferVariable();
    }
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    auto transfer_wall_temp = !parameters().get<std::string>(PARAM_VAR_T).empty();
    auto transfer_wall_heat_flux = !parameters().get<std::string>(PARAM_VAR_HF).empty();
    if (transfer_wall_temp)
    {
      if (transfer_wall_heat_flux)
      {
        syncToOpenFoam<SyncVariables::Both>();
      }
      else
      {
        syncToOpenFoam<SyncVariables::WallTemperature>();
      }
    }
    else if (transfer_wall_heat_flux)
    {
      syncToOpenFoam<SyncVariables::WallHeatFlux>();
    }

    for (auto & foam_bc : _foam_bcs)
    {
      foam_bc->imposeBoundaryCondition();
    }
  }
}

template <FoamProblem::SyncVariables sync_vars>
void
FoamProblem::syncFromOpenFoam()
{
  constexpr bool transfer_wall_temp =
      (sync_vars == SyncVariables::WallTemperature) || (sync_vars == SyncVariables::Both);
  constexpr bool transfer_wall_heat_flux =
      (sync_vars == SyncVariables::WallHeatFlux) || (sync_vars == SyncVariables::Both);

  // Find the relevant MOOSE variables to transfer values into.
  MooseVariableFieldBase * wall_temp_var;
  if constexpr (transfer_wall_temp)
  {
    wall_temp_var = getConstantMonomialVariableFromParameters(PARAM_VAR_FOAM_T);
  }
  MooseVariableFieldBase * wall_heat_flux_var;
  if constexpr (transfer_wall_heat_flux)
  {
    wall_heat_flux_var = getConstantMonomialVariableFromParameters(PARAM_VAR_FOAM_HF);
  }

  auto & mesh = this->mesh();
  auto subdomains = mesh.getSubdomainList();
  // Vectors to copy OpenFOAM quantities into.
  std::vector<Real> wall_temp;
  std::vector<Real> wall_heat_flux;

  // The number of elements in each subdomain of the mesh
  // Allocate an extra element as we'll accumulate these counts later
  std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    if constexpr (transfer_wall_temp)
    {
      auto n_added = _solver.appendPatchTemperatures(subdomains[i], wall_temp);
      patch_counts[i] = n_added;
    }
    if constexpr (transfer_wall_heat_flux)
    {
      auto n_added = _solver.wallHeatFlux(subdomains[i], wall_heat_flux);
      patch_counts[i] = n_added;
    }
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
      if constexpr (transfer_wall_temp)
      {
        auto & sys = wall_temp_var->sys();
        auto dof_t = elem_ptr->dof_number(sys.number(), wall_temp_var->number(), 0);
        sys.solution().set(dof_t, wall_temp[elem]);
      }
      if constexpr (transfer_wall_heat_flux)
      {
        auto & sys = wall_heat_flux_var->sys();
        auto dof_dt = elem_ptr->dof_number(sys.number(), wall_heat_flux_var->number(), 0);
        sys.solution().set(dof_dt, wall_heat_flux[elem]);
      }
    }
  }
  if constexpr (transfer_wall_temp)
  {
    wall_temp_var->sys().solution().close();
  }
  if constexpr (transfer_wall_heat_flux)
  {
    wall_heat_flux_var->sys().solution().close();
  }
}

template <FoamProblem::SyncVariables sync_vars>
void
FoamProblem::syncToOpenFoam()
{
  constexpr bool transfer_wall_temp =
      (sync_vars == SyncVariables::WallTemperature) || (sync_vars == SyncVariables::Both);
  constexpr bool transfer_wall_heat_flux =
      (sync_vars == SyncVariables::WallHeatFlux) || (sync_vars == SyncVariables::Both);

  // Vectors to copy MOOSE quantities into.
  std::vector<Real> wall_temp;
  std::vector<Real> wall_heat_flux;

  // Find the relevant MOOSE variables to transfer values from.
  MooseVariableFieldBase * wall_temp_var;
  if constexpr (transfer_wall_temp)
  {
    wall_temp_var = getConstantMonomialVariableFromParameters(PARAM_VAR_T);
  }
  MooseVariableFieldBase * wall_heat_flux_var;
  if constexpr (transfer_wall_heat_flux)
  {
    wall_heat_flux_var = getConstantMonomialVariableFromParameters(PARAM_VAR_HF);
  }

  auto & mesh = this->mesh();
  auto subdomains = mesh.getSubdomainList();

  // The number of elements in each subdomain of the mesh
  // Allocate an extra element as we'll accumulate these counts later
  std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    patch_counts[i] = _solver.patchSize(subdomains[i]);
  }
  std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);

  // Retrieve the values from MOOSE for each boundary we're transferring across.
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  for (auto i = 0U; i < subdomains.size(); ++i)
  {
    // Vectors to hold quantities copied from MOOSE mesh.
    std::vector<double> moose_T;
    std::vector<double> moose_hf;

    // Set the face temperatures on the OpenFOAM mesh.
    for (size_t elem = patch_counts[i]; elem < patch_counts[i + 1]; ++elem)
    {
      auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
      assert(elem_ptr);
      if constexpr (transfer_wall_temp)
      {
        moose_T.emplace_back(variableValueAtElement(elem_ptr, wall_temp_var));
      }
      if constexpr (transfer_wall_heat_flux)
      {
        moose_hf.emplace_back(variableValueAtElement(elem_ptr, wall_heat_flux_var));
      }
    }
    // Copy the values from the MOOSE temperature vector into OpenFOAM's
    if constexpr (transfer_wall_temp)
    {
      _solver.setPatchTemperatures(subdomains[i], moose_T);
    }
    if constexpr (transfer_wall_heat_flux)
    {
      _solver.setPatchNegativeHeatFlux(subdomains[i], moose_hf);
    }
  }
}

MooseVariableFieldBase *
FoamProblem::getConstantMonomialVariableFromParameters(const std::string & parameter_name)
{
  auto variable_name = parameters().get<std::string>(parameter_name);
  auto * var = &getVariable(0, variable_name);
  if (!is_constant_monomial(*var))
  {
    mooseError("variable assigned to parameter '",
               parameter_name,
               "' must have:\n"
               "  family = MONOMIAL\n"
               "  order = CONSTANT\n");
  }
  return var;
}

void
FoamProblem::verifyFoamVariables()
{
  // Create table summarising FoamVariables
  VariadicTable<std::string, std::string, std::string> vt({
      "FoamVariable name",
      "Type",
      "Foam variable",
  });
  for (auto & var : _foam_variables)
  {
    vt.addRow(var->name(), var->type(), var->foamVariable());
  }
  vt.print(_console);
}

// Create comma separated list from vector
template <typename StrType>
inline std::string
listFromVector(std::vector<StrType> vec, StrType sep = ", ")
{
  if (vec.size() == 0)
    return std::string();
  else if (vec.size() == 1)
    return vec.at(0);

  std::string str;
  auto binary_op = [&](const std::string & acc, const std::string & it) { return acc + sep + it; };
  std::accumulate(vec.begin(), vec.end(), str, binary_op);
  return str;
}

void
FoamProblem::verifyFoamBCs()
{
  // Get list of all variables used by all BCs
  std::vector<std::string> variables(_foam_bcs.size());
  for (auto & bc : _foam_bcs)
    variables.push_back(bc->foamVariable());

  std::set<std::string> unique_vars(variables.begin(), variables.end());

  // Create table for printing BC information
  VariadicTable<std::string, std::string, std::string, std::string, std::string> vt({
      "FoamBC name",
      "Type",
      "Foam variable",
      "MOOSE field",
      "Boundaries",
  });

  for (auto var : unique_vars)
  {
    if (var.empty())
      continue;

    // create list of all boundaries where bc has been applied for var
    std::vector<SubdomainName> used_bcs;
    for (auto & bc : _foam_bcs)
    {
      if (bc->foamVariable() == var)
      {
        auto && boundary = bc->boundary();
        used_bcs.insert(used_bcs.end(), boundary.begin(), boundary.end());
        // List info about BC
        vt.addRow(bc->name(),
                  bc->type(),
                  bc->foamVariable(),
                  bc->mooseVariable(),
                  listFromVector(boundary));
      }
    }

    // Find duplicates
    auto unique_bc = std::unique(used_bcs.begin(), used_bcs.end());
    if (unique_bc != used_bcs.end())
      mooseError("Imposed FoamBC has duplicated boundary '",
                 *unique_bc,
                 "' for foam variable '",
                 var,
                 "'");

    // Add table entry for boundaries which do no have BC for variable
    std::vector<SubdomainName> unused_bcs;
    for (auto bc : _mesh.getSubdomainNames(_foam_mesh->getSubdomainList()))
    {
      auto it = std::find(used_bcs.begin(), used_bcs.end(), bc);
      if (it == used_bcs.end())
        unused_bcs.push_back(bc);
    }
    if (unused_bcs.size() > 0)
      vt.addRow("", "UnusedBoundaries", "", "", listFromVector(unused_bcs));
  }
  vt.print(_console);
}
