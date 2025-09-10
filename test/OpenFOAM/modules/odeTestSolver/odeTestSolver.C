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

#include "dimensionSet.H"
#include "dimensionSets.H"
#include "dimensionedType.H"
#include "fvmDdt.H"
#include "odeTestSolver.H"
#include "fvcSurfaceIntegrate.H"
#include "fvMeshMover.H"
#include "localEulerDdtScheme.H"
#include "addToRunTimeSelectionTable.H"
#include "fvmLaplacian.H"
#include "fvConstraints.H"
#include "scalar.H"
#include "volFieldsFwd.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(odeTestSolver, 0);
addToRunTimeSelectionTable(solver, odeTestSolver, fvMesh);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

bool
Foam::solvers::odeTestSolver::dependenciesModified() const
{
  return runTime.controlDict().modified();
}

bool
Foam::solvers::odeTestSolver::read()
{
  solver::read();

  maxDeltaT_ = runTime.controlDict().found("maxDeltaT")
                   ? runTime.controlDict().lookup<scalar>("maxDeltaT", runTime.userUnits())
                   : vGreat;

  return true;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solvers::odeTestSolver::odeTestSolver(fvMesh & mesh)
  : solver(mesh),
    T_(IOobject("T", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE), mesh),
    kappa_(IOobject("kappa", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE),
           mesh,
           dimensionedScalar(dimensionSet(0, -2, 0, 0, 0), 1.)),
    T(T_),
    kappa(kappa_)
{
  // Read the controls
  read();
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::odeTestSolver::~odeTestSolver() {}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar
Foam::solvers::odeTestSolver::maxDeltaT() const
{
  return min(fvModels().maxDeltaT(), maxDeltaT_);
}

void
Foam::solvers::odeTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}

void
Foam::solvers::odeTestSolver::moveMesh()
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
Foam::solvers::odeTestSolver::motionCorrector()
{
}

void
Foam::solvers::odeTestSolver::prePredictor()
{
}

void
Foam::solvers::odeTestSolver::momentumPredictor()
{
}

void
Foam::solvers::odeTestSolver::thermophysicalPredictor()
{
  fvModels().correct();

  while (pimple.correctNonOrthogonal())
  {
    // solved ODE where T' = 1000t to verify different time schemes
    dimensionedScalar C{dimensionSet(0, 0, -1, 1, 0), 1000 * mesh_.time().userTimeValue()};
    fvScalarMatrix TEqn(Foam::fvm::ddt(T_) - C);

    fvConstraints().constrain(TEqn);
    TEqn.solve();
    fvConstraints().constrain(T_);
  }
}

void
Foam::solvers::odeTestSolver::pressureCorrector()
{
}

void
Foam::solvers::odeTestSolver::postCorrector()
{
}

void
Foam::solvers::odeTestSolver::postSolve()
{
}

// ************************************************************************* //
