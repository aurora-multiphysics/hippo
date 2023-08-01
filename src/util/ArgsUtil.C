#include "ArgsUtil.h"
cArgs::cArgs(std::string const & name) : data_{name}, offsets_{0} { data_.append(1, '\0'); }

void
cArgs::push_arg(std::string const & arg)
{
  offsets_.push_back(data_.size());
  data_.append(arg);
  data_.append(1, '\0');
}

int
cArgs::get_argc()
{
  return offsets_.size();
}

/* WARNINNG: This points into the string so
   1. will be invalid once instance of class is out of scope
   2. the pointers will (probably) not be correct if anything else is pushed
   after the call to get_argv
*/
std::vector<char *>
cArgs::get_argv()
{
  std::vector<char *> argv;
  argv.reserve(offsets_.size());
  for (auto const & offset : offsets_)
  {
    argv.push_back(data_.data() + offset);
  }
  return argv;
}
