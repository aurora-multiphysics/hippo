#pragma once

#include <string>
#include <vector>

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

  int & get_argc() { return _argc; }

  /* WARNING: This points into the string so
     1. will be invalid once instance of class is out of scope
     2. the pointers will (probably) not be correct if anything else is pushed
        after the call to get_argv
  */
  // std::vector<char *> get_argv();
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

// class cArgs {
// public:
//   /* name is name of solver aka argv[0] */
//   cArgs(std::string const &name);

//   /* push an argument on to the list */
//   void push_arg(std::string const &arg);

//   int &get_argc();

//   /* WARNING: This points into the string so
//      1. will be invalid once instance of class is out of scope
//      2. the pointers will (probably) not be correct if anything else is
//      pushed
//         after the call to get_argv
//   */
//   std::vector<char *> get_argv();
//   char **&get_argv_ptr() { return argv_; }

// private:
//   std::string data_;
//   std::vector<std::size_t> offsets_;
//   int argc_{0};
//   char *data_ptr_;
//   char **argv_;
// };
// Local Variables:
// mode: c++
// End:
