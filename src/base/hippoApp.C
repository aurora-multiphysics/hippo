#include "hippoApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
hippoApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

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
