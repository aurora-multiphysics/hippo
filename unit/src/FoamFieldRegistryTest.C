#include "FoamFieldRegistry.h"
#include "TestUtils.h"
#include "gtest/gtest.h"

namespace
{
using FoamFieldRegistryTest = HippoTest::ErrorFixture;

TEST_F(FoamFieldRegistryTest, SeparateAccessorsShareStorage)
{
  // Create Registry with two accessors that reference the same field
  Foam::scalarField field(2, 1.);
  FoamFieldRegistry registry;
  auto & first =
      registry.add("FoamVariables/T", std::make_unique<FoamFieldAccessor<Foam::scalar>>(field));
  auto & second =
      registry.add("FoamBCs/T", std::make_unique<FoamFieldAccessor<Foam::scalar>>(field));

  EXPECT_NE(&first, &second);
  // The address of object returned by registry should match the created accessor
  EXPECT_EQ(&registry.getFieldAccessor<Foam::scalar>("FoamVariables/T"), &first);
  EXPECT_EQ(&registry.getFieldAccessor<Foam::scalar>("FoamBCs/T"), &second);

  // Changing the field should change the reference field
  first.internalField()[0] = 5.;
  EXPECT_EQ(second.internalField()[0], 5.);
}

TEST_F(FoamFieldRegistryTest, DuplicateNamesAndWrongTypesError)
{
  FoamFieldRegistry registry;
  registry.add("field", std::make_unique<FoamFieldAccessor<Foam::scalar>>());

  // Cannot add the same key twice to registry
  HippoTest::expectError(
      [&]() { registry.add("field", std::make_unique<FoamFieldAccessor<Foam::scalar>>()); },
      "already registered");

  // Cannot access field using key if registered field has a different type
  HippoTest::expectError([&]() { registry.getFieldAccessor<Foam::vector>("field"); },
                         "incompatible field type");
  // Trying to accesss missing keys produces an error
  HippoTest::expectError([&]() { registry.getFieldAccessor<Foam::scalar>("missing"); },
                         "not registered");
}

// The exposing object name, not the underlying volume-field name, is registered.
TEST_F(FoamFieldRegistryTest, VolumeFieldNameDoesNotReserveObjectName)
{

  // Create standalone OpenFOAM mesh
  Foam::Time runtime(Foam::fileName("."), Foam::fileName("registry_unit_test"), false);
  Foam::fvMesh mesh(
      Foam::IOobject("mesh", "0", runtime, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE),
      Foam::pointField(),
      Foam::faceList(),
      Foam::labelList(),
      Foam::labelList(),
      false);

  // Create field
  Foam::VolField<Foam::scalar> field(
      Foam::IOobject("T", "0", mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE),
      mesh,
      Foam::dimensioned<Foam::scalar>("value", Foam::dimless, 1.));

  // Create registry with two keys referencing the same VolField
  FoamFieldRegistry registry;
  auto & temperature =
      registry.add("temperature", std::make_unique<FoamFieldAccessor<Foam::scalar>>(field));
  auto & copy =
      registry.add("temperature_copy", std::make_unique<FoamFieldAccessor<Foam::scalar>>(field));

  EXPECT_NE(&temperature, &copy);
  // Checks the underlying field has the same address
  EXPECT_EQ(&temperature.internalField(), &copy.internalField());
  // Check underlying OpenFOAM field name is not stored
  EXPECT_FALSE(registry.contains("T"));
}
} // namespace
