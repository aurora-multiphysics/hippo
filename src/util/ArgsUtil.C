#include "ArgsUtil.h"

namespace Hippo
{
cArgs::cArgs(std::string const & name)
{
  // Let's try to avoid reallocations that would invalidate pointers by
  // reserving a large-ish chunk of memory.
  // We could still run into issues if the strings are reallocated, so
  // we should try to avoid mutating the strings once they're added.
  _argv.reserve(256);
  _argv.emplace_back(name);
  update_c_argv();
}

void
cArgs::push_arg(std::string const & arg)
{
  _argv.emplace_back(arg);
  update_c_argv();
}
}
