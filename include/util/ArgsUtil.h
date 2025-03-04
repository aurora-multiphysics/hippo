#pragma once

#include <string>
#include <vector>

namespace Hippo
{
/**
 * Utility class to manage a list of arguments to pass to OpenFOAM.
 *
 * OpenFOAM's argList takes argc and argv and initialises an OpenFOAM application,
 * including MPI workers and the case directory. OpenFOAM does not take a copy of
 * these arguments. As we're not passing in the actual argc/argv (we're constructing
 * our own based on options in a MOOSE input file), we need a place to keep the
 * strings alive.
 *
 * This class's purpose is to keep the list of strings alive and provide an argv
 * style pointer to pass to OpenFOAM. You should keep the instance of this class
 * alive as long as OpenFOAM is doing things, as OpenFOAM assumes argc/argv are
 * always available.
 */
class cArgs
{
public:
  // The name is the name of solver aka argv[0]
  cArgs(std::string const & name);

  // Push an argument on to the list
  void push_arg(std::string const & arg);

  int & get_argc() { return _argc; }

  /* WARNING: This points into the string so
     1. will be invalid once instance of class is out of scope
     2. the pointers will (probably) not be correct if anything else is pushed
        after the call to get_argv
  */
  char **& get_argv_ptr() { return _argv_ptr; }

private:
  std::vector<std::string> _argv;
  std::vector<char *> _c_argv;
  int _argc{0};
  char ** _argv_ptr = nullptr;

  void update_c_argv()
  {
    _c_argv.clear();
    for (auto & arg : _argv)
    {
      _c_argv.emplace_back(arg.data());
    }
    _c_argv.push_back(nullptr);
    _argc = static_cast<int>(_argv.size());
    _argv_ptr = _c_argv.data();
  }
};
}
