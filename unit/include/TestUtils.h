#pragma once

#include "Moose.h"
#include "MooseError.h"
#include "gtest/gtest.h"
#include <string>
#include <utility>

namespace HippoTest
{
/// Enable exception-based MOOSE errors for this scope, restoring the previous setting on exit.
/// Nested guards are supported; like the underlying global flag, this is not thread-local.
class ScopedThrowOnError
{
public:
  ScopedThrowOnError() : _previous(Moose::_throw_on_error) { Moose::_throw_on_error = true; }
  ~ScopedThrowOnError() { Moose::_throw_on_error = _previous; }

  ScopedThrowOnError(const ScopedThrowOnError &) = delete;
  ScopedThrowOnError & operator=(const ScopedThrowOnError &) = delete;

private:
  const bool _previous;
};

/// Check an in-process MOOSE error and a diagnostic substring, without a death-test subprocess.
/// Only the expected exception type is caught; unrelated exceptions remain test failures.
template <typename Action>
void
expectError(Action && action, const std::string & message)
{
  ScopedThrowOnError throw_on_error;
  try
  {
    std::forward<Action>(action)();
    ADD_FAILURE() << "Expected a MOOSE error containing: " << message;
  }
  catch (const MooseRuntimeError & error)
  {
    EXPECT_NE(std::string(error.what()).find(message), std::string::npos) << error.what();
  }
}

/// Keep MOOSE errors catchable throughout fixture construction, setup, execution, and teardown.
class ErrorFixture : public ::testing::Test
{
private:
  ScopedThrowOnError _throw_on_error;
};
} // namespace HippoTest
