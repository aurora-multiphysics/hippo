#pragma once
#include <unordered_map>
#include <memory>
#include <string>

class TransferContainerBase
{
public:
  virtual ~TransferContainerBase() = default;
};

enum class TransferFieldType
{
  Scalar,
  Vector,
  Tensor
};

enum class TransferQuantity
{
  Value,
  Gradient,
  HeatFlux
};

struct TransferKey
{
  std::string name;
  TransferFieldType type;
  TransferQuantity quantity;

  bool operator==(const TransferKey & other) const
  {
    return name == other.name && type == other.type && quantity == other.quantity;
  }
};

struct TransferKeyHash
{
  std::size_t operator()(const TransferKey & key) const
  {
    std::size_t seed = 0;

    auto combine = [&seed](std::size_t hash)
    { seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2); };

    combine(std::hash<std::string>{}(key.name));
    combine(std::hash<int>{}(static_cast<int>(key.type)));
    combine(std::hash<int>{}(static_cast<int>(key.quantity)));

    return seed;
  }
};
using TransferRegistry =
    std::unordered_map<TransferKey, std::unique_ptr<TransferContainerBase>, TransferKeyHash>;
