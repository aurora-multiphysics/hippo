#pragma once

#include "multiphaseEuler.H"

namespace Foam
{
namespace solvers
{
/** A real phase system with prescribed fields and no flow evolution. */
class multiphaseTransferTestSolver : public multiphaseEuler
{
public:
  TypeName("multiphaseTransferTest");
  explicit multiphaseTransferTestSolver(fvMesh & mesh);

  void preSolve() override {}
  void moveMesh() override {}
  void motionCorrector() override {}
  void prePredictor() override {}
  void momentumTransportPredictor() override {}
  void thermophysicalTransportPredictor() override {}
  void momentumPredictor() override {}
  void thermophysicalPredictor() override;
  void pressureCorrector() override {}
  void momentumTransportCorrector() override {}
  void thermophysicalTransportCorrector() override {}
  void postSolve() override {}
};
}
}
