#pragma once

#include "baseTestSolver.H"
#include "volFields.H"

namespace Foam
{
namespace solvers
{
// Holds native field storage without changing it during solver execution.
class fieldTransferTestSolver : public baseTestSolver
{
  volScalarField _transfer_field;

public:
  TypeName("fieldTransferTest");
  explicit fieldTransferTestSolver(fvMesh & mesh);
};
}
}
