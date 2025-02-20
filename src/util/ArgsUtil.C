#include "ArgsUtil.h"

#include <cassert>

cArgs::cArgs(std::string const & name)
{
  // Let's try to avoid reallocations that would invalidate pointers.
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

// cArgs::cArgs(std::string const &name)
//     : data_{name}, offsets_{0}, argc_(1), data_ptr_(&data_[0]),
//       argv_(&data_ptr_) {
//   data_.append(1, '\0');
// }

// void cArgs::push_arg(std::string const &arg) {
//   offsets_.push_back(data_.size());
//   data_.append(arg);
//   data_.append(1, '\0');
//   ++argc_;
// }

// int &cArgs::get_argc() {
//   assert(argc_ == offsets_.size());
//   return argc_;
// }

// /* WARNINNG: This points into the string so
//    1. will be invalid once instance of class is out of scope
//    2. the pointers will (probably) not be correct if anything else is pushed
//    after the call to get_argv
// */
// std::vector<char *> cArgs::get_argv() {
//   std::vector<char *> argv;
//   argv.reserve(offsets_.size());
//   for (auto const &offset : offsets_) {
//     argv.push_back(data_.data() + offset);
//   }
//   return argv;
// }
