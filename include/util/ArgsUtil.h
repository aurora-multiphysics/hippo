#pragma once

#include <vector>
#include <string>

/*
   Utility class to manage a list of arguments to pass to openfoam
   args/argv style
   TODO: Look at openFOAM args class to see if there is a better way
*/

class cArgs
{
public:
  /* name is name of solver aka argv[0] */
  cArgs(std::string const & name);

  /* push an argument on to the list */
  void push_arg(std::string const & arg);

  int get_argc();

  /* WARNINNG: This points into the string so
     1. will be invalid once instance of class is out of scope
     2. the pointers will (probably) not be correct if anything else is pushed
        after the call to get_argv
  */
  std::vector<char *> get_argv();

private:
  std::string data_;
  std::vector<std::size_t> offsets_;
};
// Local Variables:
// mode: c++
// End:
