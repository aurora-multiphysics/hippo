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

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();

  // Deprecated parameters to set variables to read from/write to.
  // Note that these can be omitted or point to the same variable to save
  // memory.
  params.addDeprecatedParam<std::string>("foam_heat_flux",
                                         "The name of the aux variable to write the "
                                         "OpenFOAM wall heat flux into.",
                                         "Use FoamFunctionObject in the new [FoamVariables] block");
  params.addDeprecatedParam<std::string>("foam_temp",
                                         "The name of the aux variable to write the "
                                         "OpenFOAM boundary temperature into.",
                                         "Use FoamVariableField in the new [FoamVariables] block");
  params.addDeprecatedParam<std::string>(
      "heat_flux",
      "The name of the aux variable to set the OpenFOAM wall heat flux from.",
      "Use FoamFixedGradientBC in the new [FoamBCs] block");
  params.addDeprecatedParam<std::string>("temp",
                                         "The name of the aux variable to set the "
                                         "OpenFOAM boundary temperature from.",
                                         "Use FoamFixedValueBC in the new [FoamBCs] block");
  return params;
}

bool
FoamProblem::hasOldBCSyntax()
{
  return !parameters().get<std::string>("temp").empty() ||
         !parameters().get<std::string>("heat_flux").empty();
}

bool
FoamProblem::hasOldVariableSyntax()
{
  return !parameters().get<std::string>("foam_temp").empty() ||
         !parameters().get<std::string>("foam_heat_flux").empty();
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
    return;

  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {
    // Loop of shadowed variables and perform transfer
    for (auto & var : _foam_variables)
    {
      var->transferVariable();
    }
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    for (auto & foam_bc : _foam_bcs)
    {
      foam_bc->imposeBoundaryCondition();
    }
  }
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
  // Check BC
  for (auto & bc : _foam_bcs)
    bc->initialSetup();

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
      "Moose variable",
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
