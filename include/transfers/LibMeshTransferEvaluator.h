#pragma once

#include "fvCFD_moose.h"

#include "MooseEnum.h"

class LibMeshVariableNearestLocationEvaluator
{
public:
  LibMeshVariableNearestLocationEvaluator(const MooseEnum & mode,
                                          const Foam::pointField & foam_points);

  template <typename T>
  Foam::Field<T> evaluate(const std::string & var_name);
};
