#include "fieldTransferTestSolver.H"
#include "addToRunTimeSelectionTable.H"

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(fieldTransferTestSolver, 0);
addToRunTimeSelectionTable(solver, fieldTransferTestSolver, fvMesh);

fieldTransferTestSolver::fieldTransferTestSolver(fvMesh & mesh)
  : baseTestSolver(mesh),
    _transfer_field(
        IOobject(
            "transferField", mesh.time().name(), mesh, IOobject::MUST_READ, IOobject::AUTO_WRITE),
        mesh)
{
  read();
}
}
}
