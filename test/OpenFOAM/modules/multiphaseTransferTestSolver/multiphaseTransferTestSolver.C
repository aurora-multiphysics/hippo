#include "multiphaseTransferTestSolver.H"
#include "addToRunTimeSelectionTable.H"

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(multiphaseTransferTestSolver, 0);
addToRunTimeSelectionTable(solver, multiphaseTransferTestSolver, fvMesh);
}
}

Foam::solvers::multiphaseTransferTestSolver::multiphaseTransferTestSolver(fvMesh & mesh)
  : multiphaseEuler(mesh)
{
  thermophysicalPredictor();
}

void
Foam::solvers::multiphaseTransferTestSolver::thermophysicalPredictor()
{
  const dimensionedScalar factor(dimTemperature / dimArea, time().userTimeValue());
  const auto & centres = mesh_.C();
  forAll(phases_, phasei)
  {
    auto & thermo = phases_[phasei].thermo();
    auto & temperature = thermo.T();
    // Field algebra includes both cell and face centres. Forced assignment preserves
    // the analytical values on fixedValue patches as well as in the internal field.
    temperature == dimensionedScalar(dimTemperature, 300. + 20. * phasei) +
                       (phasei + 1.) * factor *(centres.component(0) * centres.component(1) +
                                                centres.component(1) * centres.component(2) +
                                                centres.component(2) * centres.component(0));
    // Include the equation of state contribution to enthalpy.
    thermo.he() == thermo.he(p_, temperature);
    thermo.correct();
  }
  fluid_.correctThermophysicalTransport();
}
