#include "BoundaryTransferContainer.h"
#include "Moose.h"
#include "MooseError.h"
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

template <typename Callable>
void
expectContainerError(Callable action, const std::string & message)
{
  try
  {
    action();
    FAIL() << "Expected a container error containing: " << message;
  }
  catch (const MooseRuntimeError & error)
  {
    EXPECT_NE(std::string(error.what()).find(message), std::string::npos) << error.what();
  }
}

template <typename T>
class BoundaryTransferContainerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    _previous_throw_on_error = Moose::_throw_on_error;
    Moose::_throw_on_error = true;
  }

  void TearDown() override { Moose::_throw_on_error = _previous_throw_on_error; }

private:
  bool _previous_throw_on_error = false;
};

using FieldTypes = ::testing::Types<Foam::scalar, Foam::vector, Foam::tensor>;
TYPED_TEST_SUITE(BoundaryTransferContainerTest, FieldTypes);

TYPED_TEST(BoundaryTransferContainerTest, FieldMapBorrowsStorage)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::FieldMap fields;
  // Create a two-entry wall field with an initial value of the tested type.
  fields.emplace("wall", typename Container::Field(2, sampleValue<TypeParam>(1)));
  Container container(fields);
  auto & original = fields.at("wall");
  auto & borrowed = container.boundaryField("wall");

  // Both references access the same field; writes through either are visible through the other.
  EXPECT_EQ(&borrowed, &original);
  borrowed[0] = sampleValue<TypeParam>(10);
  EXPECT_TRUE(original[0] == sampleValue<TypeParam>(10));
  original[1] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(borrowed[1] == sampleValue<TypeParam>(20));
}

// Tests constructor with reference map
TYPED_TEST(BoundaryTransferContainerTest, ReferenceMapBorrowsStorage)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::Field field(2, sampleValue<TypeParam>(1));
  typename Container::FieldRefMap references{{"wall", std::ref(field)}};
  Container container(references);
  auto & borrowed = container.boundaryField("wall");

  // Both references access the same field; writes through either are visible through the other.
  EXPECT_EQ(&borrowed, &field);
  borrowed[0] = sampleValue<TypeParam>(10);
  EXPECT_TRUE(field[0] == sampleValue<TypeParam>(10));
  field[1] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(borrowed[1] == sampleValue<TypeParam>(20));
}

// Append missing boundaries and accept existing names that reference the same field object.
TYPED_TEST(BoundaryTransferContainerTest, MergeAcceptsIdenticalReferences)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::Field wall(2, sampleValue<TypeParam>(1));
  typename Container::Field inlet(3, sampleValue<TypeParam>(10));
  typename Container::FieldRefMap original{{"wall", std::ref(wall)}};
  Container container(original);
  typename Container::FieldRefMap additions{{"inlet", std::ref(inlet)}, {"wall", std::ref(wall)}};

  container.addBoundaries(additions);
  container.addBoundaries(additions);
  EXPECT_EQ(&container.boundaryField("wall"), &wall);
  EXPECT_EQ(&container.boundaryField("inlet"), &inlet);
  EXPECT_TRUE(wall[0] == sampleValue<TypeParam>(1));
  EXPECT_TRUE(inlet[0] == sampleValue<TypeParam>(10));
}

// Reject a boundary name referencing a different field object, even with identical values.
TYPED_TEST(BoundaryTransferContainerTest, ConflictingBatchLeavesRegistrationUnchanged)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::Field wall(2, sampleValue<TypeParam>(1));
  typename Container::Field other_wall(wall);
  typename Container::Field inlet(2, sampleValue<TypeParam>(10));
  typename Container::FieldRefMap original{{"wall", std::ref(wall)}};
  Container container(original);
  // The new boundary sorts before the conflict, exposing any partial insertion.
  typename Container::FieldRefMap additions{{"inlet", std::ref(inlet)},
                                            {"wall", std::ref(other_wall)}};

  expectContainerError([&]() { container.addBoundaries(additions); }, "different field");
  EXPECT_EQ(&container.boundaryField("wall"), &wall);
  EXPECT_TRUE(wall[0] == sampleValue<TypeParam>(1));
  expectContainerError([&]() { container.boundaryField("inlet"); }, "not found");
}

// Locked containers reject all registration calls, including identical and empty requests,
// while the underlying field values remain writable.
TYPED_TEST(BoundaryTransferContainerTest, LockRejectsAllRegistrationButAllowsValueWrites)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::Field wall(2, sampleValue<TypeParam>(1));
  typename Container::Field inlet(2, sampleValue<TypeParam>(10));
  typename Container::FieldRefMap original{{"wall", std::ref(wall)}};
  typename Container::FieldRefMap additions{{"inlet", std::ref(inlet)}};
  typename Container::FieldRefMap empty;
  Container container(original);

  EXPECT_TRUE(container.modifiable());
  container.lock();
  container.lock();
  EXPECT_FALSE(container.modifiable());
  expectContainerError([&]() { container.addBoundaries(original); }, "locked");
  expectContainerError([&]() { container.addBoundaries(additions); }, "locked");
  expectContainerError([&]() { container.addBoundaries(empty); }, "locked");
  expectContainerError([&]() { container.boundaryField("inlet"); }, "not found");
  EXPECT_EQ(&container.boundaryField("wall"), &wall);
  container.boundaryField("wall")[0] = sampleValue<TypeParam>(20);
  EXPECT_TRUE(wall[0] == sampleValue<TypeParam>(20));
}

// Empty containers and zero-length fields are valid; missing boundaries and volume access error.
TYPED_TEST(BoundaryTransferContainerTest, EmptyStorageAndInvalidAccess)
{
  using Container = BoundaryTransferContainer<TypeParam>;
  typename Container::FieldMap fields;
  typename Container::FieldRefMap references;
  typename Container::Field empty_field(0);
  Container from_fields(fields);
  Container from_references(references);

  from_fields.addBoundaries(references);
  expectContainerError([&]() { from_fields.boundaryField("missing"); }, "not found");
  expectContainerError([&]() { from_references.boundaryField("missing"); }, "not found");
  expectContainerError([&]() { from_fields.volumeField(); }, "No volume field");
  references.emplace("empty", std::ref(empty_field));
  from_fields.addBoundaries(references);
  EXPECT_EQ(&from_fields.boundaryField("empty"), &empty_field);
  EXPECT_EQ(from_fields.boundaryField("empty").size(), 0);
}
} // namespace
