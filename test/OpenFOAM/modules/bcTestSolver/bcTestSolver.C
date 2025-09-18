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

#include "dimensionSets.H"
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

bool
Foam::solvers::bcTestSolver::dependenciesModified() const
{
  return runTime.controlDict().modified();
}

bool
Foam::solvers::bcTestSolver::read()
{
  solver::read();

  maxDeltaT_ = runTime.controlDict().found("maxDeltaT")
                   ? runTime.controlDict().lookup<scalar>("maxDeltaT", runTime.userUnits())
                   : vGreat;

  return true;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
// Solver based on solid.C module
Foam::solvers::bcTestSolver::bcTestSolver(fvMesh & mesh, autoPtr<solidThermo> thermoPtr)
  : solver(mesh),

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

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::bcTestSolver::~bcTestSolver() {}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar
Foam::solvers::bcTestSolver::maxDeltaT() const
{
  return min(fvModels().maxDeltaT(), maxDeltaT_);
}

void
Foam::solvers::bcTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}

void
Foam::solvers::bcTestSolver::moveMesh()
{
  if (pimple.firstIter() || pimple.moveMeshOuterCorrectors())
  {
    if (!mesh_.mover().solidBody())
    {
      FatalErrorInFunction << "Region " << name() << " of type " << type()
                           << " does not support non-solid body mesh motion" << exit(FatalError);
    }

    mesh_.move();
  }
}

void
Foam::solvers::bcTestSolver::motionCorrector()
{
}

void
Foam::solvers::bcTestSolver::prePredictor()
{
}

void
Foam::solvers::bcTestSolver::momentumPredictor()
{
}

void
Foam::solvers::bcTestSolver::thermophysicalPredictor()
{
  fvScalarMatrix TEqn(fvm::laplacian(thermo_.kappa(), T));

  fvConstraints().constrain(TEqn);
  TEqn.solve();
  fvConstraints().constrain(T_);
}

void
Foam::solvers::bcTestSolver::pressureCorrector()
{
}

void
Foam::solvers::bcTestSolver::postCorrector()
{
}

void
Foam::solvers::bcTestSolver::postSolve()
{
}

// ************************************************************************* //
