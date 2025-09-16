#include "ActionFactory.h"
#include "hippoApp.h"

#include <AppFactory.h>
#include <ModulesApp.h>
#include <Moose.h>
#include <MooseSyntax.h>

InputParameters
hippoApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

hippoApp::hippoApp(InputParameters parameters) : MooseApp(parameters)
{
  hippoApp::registerAll(_factory, _action_factory, _syntax);
}

hippoApp::~hippoApp() {}

void
hippoApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAllObjects<hippoApp>(f, af, syntax);
  Registry::registerObjectsTo(f, {"hippoApp"});
  Registry::registerActionsTo(af, {"hippoApp"});

  /* register custom execute flags, action syntax, etc. here */

  // Add input file syntax for the [FoamVariables] block
  registerSyntaxTask("AddFoamVariableAction", "FoamVariables/*", "add_foam_variable");
  registerMooseObjectTask("add_foam_variable", FoamVariable, false);
  addTaskDependency("add_external_aux_variables", "add_foam_variable");

  // Add input file syntax for the [FoamBCs] block
  registerSyntaxTask("AddFoamBCAction", "FoamBCs/*", "add_foam_bc");
  registerMooseObjectTask("add_foam_bc", FoamBC, false);
}

void
hippoApp::registerApps()
{
  registerApp(hippoApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
hippoApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  hippoApp::registerAll(f, af, s);
}
extern "C" void
hippoApp__registerApps()
{
  hippoApp::registerApps();
}
