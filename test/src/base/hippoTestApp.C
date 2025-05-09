//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "hippoTestApp.h"
#include "hippoApp.h"
#include "Moose.h"
#include "AppFactory.h"

InputParameters
hippoTestApp::validParams()
{
  InputParameters params = hippoApp::validParams();
  return params;
}

hippoTestApp::hippoTestApp(InputParameters parameters) : MooseApp(parameters)
{
  hippoTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

hippoTestApp::~hippoTestApp() {}

void
hippoTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  hippoApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"hippoTestApp"});
    Registry::registerActionsTo(af, {"hippoTestApp"});
  }
}

void
hippoTestApp::registerApps()
{
  registerApp(hippoApp);
  registerApp(hippoTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
hippoTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  hippoTestApp::registerAll(f, af, s);
}
extern "C" void
hippoTestApp__registerApps()
{
  hippoTestApp::registerApps();
}
