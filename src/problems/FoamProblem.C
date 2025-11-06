#include "Attributes.h"
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

#include <IOobjectList.H>
#include <volFields.H>

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
  ExternalProblem::initialSetup();

  // Get FoamVariables created by the action AddFoamVariableAction
  TheWarehouse::Query query_vars = theWarehouse().query().condition<AttribSystem>("FoamVariable");
  query_vars.queryInto(_foam_variables);

  verifyFoamVariables();

  // Get FoamBCs created by the action AddFoamBCAction
  TheWarehouse::Query query_bcs = theWarehouse().query().condition<AttribSystem>("FoamBC");
  query_bcs.queryInto(_foam_bcs);

  verifyFoamBCs();

  verifyFoamPostprocessors();
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
  // Check BC
  for (auto & bc : _foam_bcs)
    bc->initialSetup();

  // Get list of all variables used by all BCs
  std::set<std::string> unique_vars;
  for (const auto & bc : _foam_bcs)
    unique_vars.insert(bc->foamVariable());

  // Create table for printing BC information
  VariadicTable<std::string, std::string, std::string, std::string, std::string> vt({
      "FoamBC name",
      "Type",
      "Foam variable",
      "Moose variable/postprocessor",
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
        bc->addInfoRow(vt);
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

    // Add table entry for boundaries which do not have a BC for variable
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

void
FoamProblem::verifyFoamPostprocessors()
{
  std::vector<Postprocessor *> pps;
  TheWarehouse::Query query_uos =
      theWarehouse().query().condition<AttribInterfaces>(Interfaces::Postprocessor);
  query_uos.queryInto(pps);

  VariadicTable<std::string, std::string, std::string> vt({
      "Foam postprocessor",
      "Type",
      "Boundaries",
  });

  for (auto pp : pps)
  {
    auto fpp = dynamic_cast<FoamPostprocessorBase *>(pp);
    if (fpp)
    {
      _foam_postprocessor.push_back(fpp);
      vt.addRow(fpp->name(), fpp->type(), listFromVector(fpp->blocks()));
    }
  }

  vt.print(_console);
}
