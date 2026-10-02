#pragma once

#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseVariableFieldBase.h"
#include "FoamMesh.h"
#include <InputParameters.h>
#include <string>

namespace Hippo
{
namespace
{
template <typename T>
std::string
getDemangleName()
{
  const char * mangled = typeid(T).name();
  int status = 0;
  std::unique_ptr<char> demangled(abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status));

  return status == 0 ? demangled.get() : mangled;
}
}

namespace internal
{
template <typename T>
inline void
copyParamFromParam(InputParameters & dst, const InputParameters & src, const std::string & name_in)
{
  // Check the parameter is available in both src and dst
  if (!src.have_parameter<T>(name_in))
    mooseError("Parameter '",
               name_in,
               "' of type ",
               getDemangleName<T>(),
               " not available in src parameters.");

  if (!dst.have_parameter<T>(name_in))
    mooseError("Parameter '",
               name_in,
               "' of type ",
               getDemangleName<T>(),
               " not available in dst parameters.");

  // Only set the parameter, if it has been set in src
  if (src.isParamSetByUser(name_in))
    dst.set<T>(name_in) = src.get<T>(name_in);
}

template <typename StrType>
inline std::string
listFromVector(std::vector<StrType> vec, const char * sep = ", ")
{
  if (vec.size() == 0)
    return std::string();
  else if (vec.size() == 1)
    return vec.at(0);

  std::string str{vec[0]};
  auto binary_op = [&](const std::string & acc, const std::string & it) { return acc + sep + it; };
  return std::accumulate(vec.begin() + 1, vec.end(), str, binary_op);
}

template <typename FoamField>
inline void
copyFieldFoamToMoose(const FoamMesh & mesh,
                     const FoamField & field,
                     MooseVariableFieldBase & moose_var,
                     SubdomainID subdomain)
{
  size_t patch_count = mesh.getPatchCount(subdomain);
  size_t patch_offset = mesh.getPatchOffset(subdomain);
  for (size_t j = 0; j < patch_count; ++j)
  {
    auto elem = patch_offset + j;
    auto elem_ptr = mesh.getElemPtr(elem + mesh.rank_element_offset);
    assert(elem_ptr);
    auto dof_t = elem_ptr->dof_number(moose_var.sys().number(), moose_var.number(), 0);
    moose_var.sys().solution().set(dof_t, field[j]);
  }
}

}
}
