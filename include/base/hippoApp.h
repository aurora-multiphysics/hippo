#pragma once

#include <MooseApp.h>

class hippoApp : public MooseApp
{
public:
  static InputParameters validParams();

  hippoApp(InputParameters parameters);
  virtual ~hippoApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

  void preBackup() override;
  void postRestore(const bool /* for_restart */) override;
};
