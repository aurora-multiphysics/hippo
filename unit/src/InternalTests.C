#include "gtest/gtest.h"
#include <string>

#include "InputParameters.h"
#include "TestUtils.h"
#include "hippoUtils.h"

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
  HippoTest::ScopedThrowOnError throw_on_error;

  InputParameters src(setupParameters());
  InputParameters dst(setupParameters());

  // Add parameters unique to each parameter set
  src.addParam<Real>("param3", "");
  dst.addParam<Real>("param4", "");

  Hippo::internal::copyParamFromParam<Real>(dst, src, "param1");
  Hippo::internal::copyParamFromParam<Real>(dst, src, "param2");

  EXPECT_FALSE(dst.isParamSetByUser("param1")) << "param1 should not be set if src is not set";
  EXPECT_FALSE(dst.isParamSetByUser("param2")) << "param2 should not be set if src is not set";

  // Set source parameters
  src.set<Real>("param1") = 4.2;
  src.set<Real>("param2") = 2.1;

  // Test copies
  Hippo::internal::copyParamFromParam<Real>(dst, src, "param1");
  EXPECT_DOUBLE_EQ(dst.get<Real>("param1"), 4.2) << "param1 doesn't match expect value";
  Hippo::internal::copyParamFromParam<Real>(dst, src, "param2");
  EXPECT_DOUBLE_EQ(dst.get<Real>("param2"), 2.1) << "param2 doesn't match expect value";

  // Missing source/destination parameters must report the corresponding error.
  HippoTest::expectError([&]() { Hippo::internal::copyParamFromParam<Real>(dst, src, "param4"); },
                         "Parameter 'param4' of type double not available in src parameters.");
  HippoTest::expectError([&]() { Hippo::internal::copyParamFromParam<Real>(dst, src, "param3"); },
                         "Parameter 'param3' of type double not available in dst parameters.");

  // Use the actual demangled type name instead of a subprocess-output regex.
  HippoTest::expectError([&]()
                         { Hippo::internal::copyParamFromParam<std::string>(dst, src, "param1"); },
                         "Parameter 'param1' of type " + Hippo::getDemangleName<std::string>() +
                             " not available in src parameters.");
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
