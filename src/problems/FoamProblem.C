#include "FoamMesh.h"
#include "FoamProblem.h"
#include "FoamSolver.h"
#include "hippoUtils.h"
#include "FoamSidePostprocessor.h"

#include <AuxiliarySystem.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <InputParameters.h>
#include <VariadicTable.h>

#include <finiteVolume/solver/solver.H>
#include <word.H>
#include <fvMesh.H>
#include <libmesh/enum_order.h>
#include <libmesh/fe_type.h>
#include <IOobjectList.H>
#include <volFields.H>

#include <algorithm>
#include <map>
#include <set>

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
    _solver(Foam::solver::New(_foam_mesh->fvMesh().time().controlDict().lookupOrDefault<Foam::word>(
                                  "solver", "fluid"),
                              _foam_mesh->fvMesh())
                .ptr()),
    _foam_variables(),
    _foam_bcs(),
    _foam_postprocessor()
{
  assert(_foam_mesh);
}

void
FoamProblem::initialSetup()
{
  verifyFoamVariables();

  verifyFoamBCs();

  verifyFoamPostprocessors();

  // All field accessors must be registered before the base setup executes INITIAL transfers.
  ExternalProblem::initialSetup();
}

void
FoamProblem::externalSolve()
{
  if (parameters().get<bool>("solve"))
  {
    if (!_solver.isDeltaTAdjustable())
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
    // Update variable values and perform enabled mirroring.
    for (auto & var : _foam_variables)
    {
      var->syncVariable();
    }
    for (auto & fpp : _foam_postprocessor)
    {
      fpp->compute();
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
  TheWarehouse::Query query = theWarehouse().query().condition<AttribSystem>("FoamVariable");
  query.queryInto(_foam_variables);

  // Patch replacement finishes during construction, before variables borrow patch storage.
  for (auto * variable : _foam_variables)
    variable->initialSetup();

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

void
FoamProblem::verifyFoamBCs()
{
  TheWarehouse::Query query = theWarehouse().query().condition<AttribSystem>("FoamBC");
  query.queryInto(_foam_bcs);

  std::map<std::string, std::set<SubdomainName>> selected;
  for (const auto * bc : _foam_bcs)
    for (const auto & boundary : bc->boundary())
      if (!selected[bc->foamVariable()].insert(boundary).second)
        mooseError("Imposed FoamBC has duplicated boundary '",
                   boundary,
                   "' for foam variable '",
                   bc->foamVariable(),
                   "'");
  for (auto * bc : _foam_bcs)
    bc->registerFieldAccessor();

  // Check BC
  for (auto & bc : _foam_bcs)
    bc->initialSetup();

  // Get list of all variables used by all BCs
  std::set<std::string> unique_vars;
  for (const auto & bc : _foam_bcs)
    unique_vars.insert(bc->foamVariable());

  // Create table for printing BC information
  VariadicTable<std::string, std::string, std::string, std::string, std::string, std::string> vt(
      {"FoamBC name",
       "Type",
       "Foam variable",
       "Moose variable/postprocessor",
       "Boundaries",
       "Patch replaced?"});

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
        auto [name, type, foam_var, moose_var, boundaries, patch_replaced] = bc->getInfoRow();
        vt.addRow(name, type, foam_var, moose_var, boundaries, patch_replaced);
      }
    }

    // Add table entry for boundaries which do not have a BC for variable
    std::vector<SubdomainName> unused_bcs;
    for (auto bc : _mesh.getSubdomainNames(_foam_mesh->getSubdomainList()))
    {
      auto it = std::find(used_bcs.begin(), used_bcs.end(), bc);
      if (it == used_bcs.end())
        unused_bcs.push_back(bc);
    }
    if (unused_bcs.size() > 0)
      vt.addRow("", "UnusedBoundaries", "", "", Hippo::internal::listFromVector(unused_bcs), "");
  }
  vt.print(_console);
}

void
FoamProblem::verifyFoamPostprocessors()
{
  std::vector<Postprocessor *> pps;
  TheWarehouse::Query query_uos =
      theWarehouse().query().condition<AttribInterfaces>(Interfaces::Postprocessor);
  query_uos.queryInto(pps);

  VariadicTable<std::string, std::string, std::string> side_vt({
      "Foam side postprocessor",
      "Type",
      "Boundaries",
  });

  for (auto pp : pps)
  {
    auto fpp = dynamic_cast<FoamPostprocessorBase *>(pp);
    if (fpp)
    {
      _foam_postprocessor.push_back(fpp);
      // Add side postprocessors to the side table
      if (auto side_fpp = dynamic_cast<FoamSidePostprocessor *>(fpp))
      {
        side_vt.addRow(side_fpp->name(),
                       side_fpp->type(),
                       Hippo::internal::listFromVector(side_fpp->boundary()));
      }
    }
  }

  side_vt.print(_console);
}
