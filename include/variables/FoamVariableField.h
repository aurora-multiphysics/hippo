#pragma once

#include "FoamVariableBase.h"

#include <volFields.H>

class FoamVariableField : public FoamVariableBase
{
public:
  explicit FoamVariableField(const InputParameters & params);
};
