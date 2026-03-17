#pragma once

#include <InputParameters.h>

namespace Hippo
{
namespace internal
{
template <typename T>
inline void
copyParamFromParam(InputParameters & dst, const InputParameters & src, const std::string & name_in)
{
  if (src.isParamValid(name_in))
    dst.set<T>(name_in) = src.get<T>(name_in);
}

template <typename StrType>
inline std::string
listFromVector(std::vector<StrType> vec, StrType sep = ", ")
{
  if (vec.size() == 0)
    return std::string();
  else if (vec.size() == 1)
    return vec.at(0);

  std::string str;
  auto binary_op = [&](const std::string & acc, const std::string & it) { return acc + sep + it; };
  std::accumulate(vec.begin(), vec.end(), str, binary_op);
  return str;
}
}
}
