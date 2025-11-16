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

#include "functionTestSolver.H"
#include "fvMeshMover.H"
#include "addToRunTimeSelectionTable.H"
#include "fvcDdt.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(functionTestSolver, 0);
addToRunTimeSelectionTable(solver, functionTestSolver, fvMesh);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

bool
Foam::solvers::functionTestSolver::dependenciesModified() const
{
  return runTime.controlDict().modified();
}

bool
Foam::solvers::functionTestSolver::read()
{
  solver::read();

  maxDeltaT_ = runTime.controlDict().found("maxDeltaT")
                   ? runTime.controlDict().lookup<scalar>("maxDeltaT", runTime.userUnits())
                   : vGreat;

  return true;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solvers::functionTestSolver::functionTestSolver(fvMesh & mesh)
  : solver(mesh),
    T_(IOobject("T", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE), mesh),
    dTdt_(IOobject("dTdt", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE),
          mesh,
          dimensionedScalar{dimTemperature / dimTime, 0.}),
    kappa_(IOobject("kappa", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE),
           mesh,
           1.),
    T(T_),
    dTdt(dTdt_),
    kappa(kappa_)

{
  // Read the controls
  read();
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::functionTestSolver::~functionTestSolver() {}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar
Foam::solvers::functionTestSolver::maxDeltaT() const
{
  return min(fvModels().maxDeltaT(), maxDeltaT_);
}

void
Foam::solvers::functionTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}

void
Foam::solvers::functionTestSolver::moveMesh()
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
Foam::solvers::functionTestSolver::motionCorrector()
{
}

void
Foam::solvers::functionTestSolver::prePredictor()
{
}

void
Foam::solvers::functionTestSolver::momentumPredictor()
{
}

void
Foam::solvers::functionTestSolver::thermophysicalPredictor()
{
  // Set T to the current time
  dimensioned<Foam::scalar> T0("T0", dimTemperature, mesh_.time().userTimeValue());
  T_ = T0;

  // compute time derivative will be 0 on the first step (even if this is stupid)
  // and 1 on all others
  dTdt_ = fvc::ddt(T_);
}

void
Foam::solvers::functionTestSolver::pressureCorrector()
{
}

void
Foam::solvers::functionTestSolver::postCorrector()
{
}

void
Foam::solvers::functionTestSolver::postSolve()
{
}

// ************************************************************************* //
