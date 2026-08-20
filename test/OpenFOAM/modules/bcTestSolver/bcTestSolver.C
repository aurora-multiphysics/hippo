/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2022-2024 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "fvMesh.H"
#include "bcTestSolver.H"
#include "fvMeshMover.H"
#include "addToRunTimeSelectionTable.H"
#include "fvConstraints.H"
#include "fvmLaplacian.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(bcTestSolver, 0);
addToRunTimeSelectionTable(solver, bcTestSolver, fvMesh);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
// Solver based on solid.C module
Foam::solvers::bcTestSolver::bcTestSolver(fvMesh & mesh, autoPtr<solidThermo> thermoPtr)
  : baseTestSolver(mesh),

    thermoPtr_(thermoPtr),
    thermo_(thermoPtr_()),

    T_(IOobject("T", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE), mesh),

    thermophysicalTransport(solidThermophysicalTransportModel::New(thermo_)),
    thermo(thermo_),
    T(T_)
{
  thermo.validate("solid", "h", "e");
}

Foam::solvers::bcTestSolver::bcTestSolver(fvMesh & mesh)
  : bcTestSolver(mesh, solidThermo::New(mesh))
{
  // Read the controls
  read();
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //
void
Foam::solvers::bcTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}

void
Foam::solvers::bcTestSolver::thermophysicalPredictor()
{
  fvScalarMatrix eEqn(fvm::laplacian(thermo_.kappa(), thermo.he()));

  eEqn.solve();

  thermo.he().write();
  thermo_.correct();
}

// ************************************************************************* //
