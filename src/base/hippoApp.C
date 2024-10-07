#include "hippoApp.h"
#include "FoamProblem.h"

#include <Moose.h>
#include <AppFactory.h>
#include <ModulesApp.h>
#include <MooseSyntax.h>

#include <cstdlib>

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

void
hippoApp::preBackup()
{
  auto problem = reinterpret_cast<FoamProblem *>(&feProblem());
  if (!problem)
  {
    mooseError("FoamProblem must be used with a Hippo App");
  }
  printf("preBackup()\n");
  problem->saveState();

  // auto problem = reinterpret_cast<FoamProblem *>(&feProblem());
  // if (!problem)
  // {
  //   mooseError("FoamProblem must be used with a Hippo App");
  // }
  // auto * foam_interface = problem->mesh().getFoamInterface();
  // foam_interface->write();

  // // std::string temp_dir = std::getenv("TMP");
  // // if (temp_dir.empty())
  // // {
  // //   temp_dir = "/tmp";
  // // }

  // auto backup_dir = fs::path("./backup");
  // if (!fs::is_directory(backup_dir))
  // {
  //   fs::create_directory(backup_dir);
  // }
  // auto time_path = foam_interface->currentTimePath();
  // auto backup_path = backup_dir / time_path.filename();
  // printf("backup %s -> %s\n", time_path.c_str(), backup_path.c_str());
  // fs::copy(
  //     time_path, backup_path, fs::copy_options::overwrite_existing |
  //     fs::copy_options::recursive);

  // // auto tid = parameters().get<THREAD_ID>("_tid");
  // // THREAD_ID tid = processor_id();
  // // auto * app =
  // //     reinterpret_cast<hippoApp
  // //     *>(AppFactory::instance().registeredObjects().at("hippoApp").get());
  // // std::vector<std::string> vol_scalar_fields;
  // // for (const auto & obj_name : app->feProblem())

  // //   _restartable_data[tid].addData(std::move(foam_data));
  // // printf("tid=%u\n", tid);
  // // printf("_restartable_data.size()=%lu\n", _restartable_data.size());

  // //   // TODO: look at 'readCheckpoint' and 'writeCheckpoint' in precice adapter.
  // //   // OR:
  // //   // _interface->getMesh().writeObject(
  // //   //     IOstream::streamFormat fmt, IOstream::versionNumber ver, IOstream::compressionType cmp, )
  // // }
}

void
hippoApp::postRestore(const bool for_restart)
{
  auto problem = reinterpret_cast<FoamProblem *>(&feProblem());
  if (!problem)
  {
    mooseError("FoamProblem must be used with a Hippo App");
  }
  printf("postRestore()\n");
  problem->loadState();

  // auto problem = reinterpret_cast<FoamProblem *>(&feProblem());
  // if (!problem)
  // {
  //   mooseError("FoamProblem must be used with a Hippo App");
  // }
  // auto * foam_interface = problem->mesh().getFoamInterface();

  // auto time_path = foam_interface->currentTimePath();
  // auto backup_dir = fs::path("./backup");
  // auto backup_path = backup_dir / time_path.filename();
  // if (!fs::is_directory(backup_path))
  // {
  //   return;
  // }
  // printf("restoring %s -> %s\n", backup_path.c_str(), time_path.c_str());
  // fs::copy(
  //     backup_path, time_path, fs::copy_options::overwrite_existing |
  //     fs::copy_options::recursive);
  // foam_interface->readTime("");
  // // std::vector<double> _;
  // // foam_interface->getWallHeatFlux(_, 0);
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
