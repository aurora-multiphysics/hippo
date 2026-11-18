#include "FoamFieldAccessor.h"
#include "TestUtils.h"
#include "gtest/gtest.h"

#include <functional>
#include <string>
#include <type_traits>

namespace
{
template <typename T>
T
sampleValue(Foam::scalar offset)
{
  if constexpr (std::is_same_v<T, Foam::scalar>)
    return offset;
  else
  {
    T value;
    for (unsigned int component = 0; component < T::nComponents; ++component)
      value[component] = offset + component;
    return value;
  }
}

template <typename T>
class FoamFieldAccessorTest : public HippoTest::ErrorFixture
{
};

using FieldTypes = ::testing::Types<Foam::scalar, Foam::vector, Foam::tensor>;
TYPED_TEST_SUITE(FoamFieldAccessorTest, FieldTypes);

TYPED_TEST(FoamFieldAccessorTest, FieldMapBorrowsStorage)
{
  using Accessor = FoamFieldAccessor<TypeParam>;
  typename Accessor::FieldMap fields;
  // Create a two-entry wall field with an initial value of the tested type.
  fields.emplace("wall", typename Accessor::Field(2, sampleValue<TypeParam>(1)));
  Accessor accessor(fields);
  auto & original = fields.at("wall");
  auto & borrowed = accessor.boundaryField("wall");

  // Both references access the same field; writes through either are visible through the other.
  EXPECT_EQ(&borrowed, &original);
  borrowed[0] = sampleValue<TypeParam>(10);
  EXPECT_TRUE(original[0] == sampleValue<TypeParam>(10));
  original[1] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(borrowed[1] == sampleValue<TypeParam>(20));
}

TYPED_TEST(FoamFieldAccessorTest, ReferenceMapBorrowsStorage)
{
  using Accessor = FoamFieldAccessor<TypeParam>;
  typename Accessor::Field field(2, sampleValue<TypeParam>(1));
  typename Accessor::FieldRefMap references{{"wall", std::ref(field)}};
  Accessor accessor(references);
  auto & borrowed = accessor.boundaryField("wall");

  // Both references access the same field; writes through either are visible through the other.
  EXPECT_EQ(&borrowed, &field);
  borrowed[0] = sampleValue<TypeParam>(10);
  EXPECT_TRUE(field[0] == sampleValue<TypeParam>(10));
  field[1] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(borrowed[1] == sampleValue<TypeParam>(20));
}

TYPED_TEST(FoamFieldAccessorTest, MissingStorageErrors)
{
  FoamFieldAccessor<TypeParam> empty;
  EXPECT_TRUE(empty.boundaryNames().empty());
  HippoTest::expectError([&]() { empty.boundaryField("missing"); }, "not found");
  HippoTest::expectError([&]() { empty.internalField(); }, "No internal field");
}

TYPED_TEST(FoamFieldAccessorTest, PartialAccessorExposesOnlySelectedBoundaries)
{
  using Accessor = FoamFieldAccessor<TypeParam>;
  typename Accessor::FieldMap fields;
  fields.emplace("wall", typename Accessor::Field(2, sampleValue<TypeParam>(1)));
  fields.emplace("inlet", typename Accessor::Field(2, sampleValue<TypeParam>(2)));

  const typename Accessor::FieldRefMap selected{{"wall", std::ref(fields.at("wall"))}};
  Accessor accessor(selected);

  // Check only wall is present in boundaryNames
  EXPECT_EQ(accessor.boundaryNames(), (std::vector<std::string>{"wall"}));
  // Check wall references the correct address
  EXPECT_EQ(&accessor.boundaryField("wall"), &fields.at("wall"));

  // Unexposed boundaries and missing internal storage report errors.
  HippoTest::expectError([&]() { accessor.boundaryField("inlet"); }, "not found");
  HippoTest::expectError([&]() { accessor.internalField(); }, "No internal field");
}

TYPED_TEST(FoamFieldAccessorTest, ConstructorsBorrowInternalAndBoundaryStorage)
{
  using Accessor = FoamFieldAccessor<TypeParam>;
  Foam::Field<TypeParam> internal(2, sampleValue<TypeParam>(1));
  typename Accessor::FieldMap fields;
  fields.emplace("wall", typename Accessor::Field(1, sampleValue<TypeParam>(2)));
  typename Accessor::FieldRefMap references{{"wall", std::ref(fields.at("wall"))}};

  // Both boundary-map constructors should reference the same internal field.
  Accessor from_fields(fields, std::ref(internal));
  Accessor from_references(references, std::ref(internal));
  for (auto * accessor : {&from_fields, &from_references})
    EXPECT_EQ(&accessor->internalField(), &internal);

  // Boundary fields should have the same address from different constructors
  EXPECT_EQ(&from_fields.boundaryField("wall"), &fields.at("wall"));
  EXPECT_EQ(&from_references.boundaryField("wall"), &fields.at("wall"));

  // Writes through one accessor are visible through the other reference.
  from_fields.internalField()[0] = sampleValue<TypeParam>(10);
  EXPECT_TRUE(from_references.internalField()[0] == sampleValue<TypeParam>(10));
}

TYPED_TEST(FoamFieldAccessorTest, InternalOnlyAccessorBorrowsStorage)
{
  Foam::Field<TypeParam> field(2, sampleValue<TypeParam>(1));
  FoamFieldAccessor<TypeParam> accessor(field);

  // Internal field address should match
  EXPECT_EQ(&accessor.internalField(), &field);

  // Writes through the accessor are visible through the original field reference.
  accessor.internalField()[0] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(field[0] == sampleValue<TypeParam>(20));

  // There should be no boundaries
  EXPECT_TRUE(accessor.boundaryNames().empty());
  HippoTest::expectError([&]() { accessor.boundaryField("wall"); }, "not found");
}

TYPED_TEST(FoamFieldAccessorTest, VolumeFieldBorrowsInternalAndBoundaryStorage)
{
  // Construct an empty mesh without reading or generating a case.
  Foam::Time runtime(Foam::fileName("."), Foam::fileName("accessor_unit_test"), false);
  Foam::fvMesh mesh(
      Foam::IOobject("mesh", "0", runtime, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE),
      Foam::pointField(),
      Foam::faceList(),
      Foam::labelList(),
      Foam::labelList(),
      false);

  // The mesh owns both empty patches, allowing boundary-name lookup to be checked.
  Foam::List<Foam::polyPatch *> patches(2);
  patches[0] = new Foam::polyPatch("wall", 0, 0, 0, mesh.poly().boundary());
  patches[1] = new Foam::polyPatch("inlet", 0, 0, 1, mesh.poly().boundary());
  mesh.addFvPatches(patches, false);

  Foam::VolField<TypeParam> field(
      Foam::IOobject("field", "0", mesh, Foam::IOobject::NO_READ, Foam::IOobject::NO_WRITE),
      mesh,
      Foam::dimensioned<TypeParam>("value", Foam::dimless, sampleValue<TypeParam>(1)));

  // Even though the field is empty we can still check address and boundary names
  FoamFieldAccessor<TypeParam> accessor(field);
  EXPECT_EQ(accessor.boundaryNames(), (std::vector<std::string>{"inlet", "wall"}));
  EXPECT_EQ(&accessor.internalField(), &field.primitiveFieldRef());
  EXPECT_EQ(&accessor.boundaryField("wall"),
            static_cast<Foam::Field<TypeParam> *>(&field.boundaryFieldRef()[0]));
  EXPECT_EQ(&accessor.boundaryField("inlet"),
            static_cast<Foam::Field<TypeParam> *>(&field.boundaryFieldRef()[1]));
  HippoTest::expectError([&]() { accessor.boundaryField("missing"); }, "not found");
}
} // namespace
