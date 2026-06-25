#include "gtest/gtest.h"
#include <string>

#include "InputParameters.h"
#include "hippoUtils.h"

#define EXPECT_THROW_MSG(statement, msg)                                                           \
  try statement catch (const std::exception & e)                                                   \
  {                                                                                                \
    const std::string err_msg{std::string(e.what()) + " vs " + msg};                               \
    EXPECT_TRUE(std::regex_search(e.what(), std::regex(msg))) << err_msg;                          \
  }

InputParameters
setupParameters()
{
  libMesh::Parameters params;
  params.insert<Real>("param1");
  params.insert<Real>("param2");

  return InputParameters(params);
}

TEST(HippoTestInternals, copyParamTest)
{
  InputParameters src(setupParameters());
  InputParameters dst(setupParameters());

  // Add parameters unique to each parameter set
  src.addParam<Real>("param3", "");
  dst.addParam<Real>("param4", "");

  // Set source parameters
  src.set<Real>("param1") = 4.2;
  src.set<Real>("param2") = 2.1;

  // Test copies
  Hippo::internal::copyParamFromParam<Real>(dst, src, "param1");
  EXPECT_DOUBLE_EQ(dst.get<Real>("param1"), 4.2) << "param1 doesn't match expect value";
  Hippo::internal::copyParamFromParam<Real>(dst, src, "param2");
  EXPECT_DOUBLE_EQ(dst.get<Real>("param2"), 2.1) << "param2 doesn't match expect value";

  // Check missing parameters results in an excection
  EXPECT_THROW_MSG(
      { Hippo::internal::copyParamFromParam<Real>(dst, src, "param4"); },
      "Parameter 'param4' of type double not found in src parameters.");
  EXPECT_THROW_MSG(
      { Hippo::internal::copyParamFromParam<Real>(dst, src, "param3"); },
      "Parameter 'param3' of type double not found in dst parameters.");

  // Check error through if the type is wrong
  EXPECT_THROW_MSG(
      { Hippo::internal::copyParamFromParam<std::string>(dst, src, "param1"); },
      "Parameter 'param1' of type .* not found in src parameters.");
}

TEST(HippoTestInternals, ListFromVectorTest)
{
  std::vector<std::string> vec1{"item 1"};
  std::vector<std::string> vec2{"item 1", "item 2"};

  // Check lists of different sizes
  EXPECT_EQ(Hippo::internal::listFromVector(std::vector<std::string>()), "");
  EXPECT_EQ(Hippo::internal::listFromVector(vec1), "item 1");
  EXPECT_EQ(Hippo::internal::listFromVector(vec2), "item 1, item 2");

  // Check separator usage
  EXPECT_EQ(Hippo::internal::listFromVector(vec2, " "), "item 1 item 2");
}
