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
#include "postprocessorTestSolver.H"
#include "fvMeshMover.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(postprocessorTestSolver, 0);
addToRunTimeSelectionTable(solver, postprocessorTestSolver, fvMesh);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

bool
Foam::solvers::postprocessorTestSolver::dependenciesModified() const
{
  return runTime.controlDict().modified();
}

bool
Foam::solvers::postprocessorTestSolver::read()
{
  solver::read();

  maxDeltaT_ = runTime.controlDict().found("maxDeltaT")
                   ? runTime.controlDict().lookup<scalar>("maxDeltaT", runTime.userUnits())
                   : vGreat;

  return true;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
// Solver based on solid.C module
Foam::solvers::postprocessorTestSolver::postprocessorTestSolver(fvMesh & mesh,
                                                                autoPtr<solidThermo> thermoPtr)
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

Foam::solvers::postprocessorTestSolver::postprocessorTestSolver(fvMesh & mesh)
  : postprocessorTestSolver(mesh, solidThermo::New(mesh))
{
  // Read the controls
  read();
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::postprocessorTestSolver::~postprocessorTestSolver() {}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::scalar
Foam::solvers::postprocessorTestSolver::maxDeltaT() const
{
  return min(fvModels().maxDeltaT(), maxDeltaT_);
}

void
Foam::solvers::postprocessorTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}

void
Foam::solvers::postprocessorTestSolver::moveMesh()
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
Foam::solvers::postprocessorTestSolver::motionCorrector()
{
}

void
Foam::solvers::postprocessorTestSolver::prePredictor()
{
}

void
Foam::solvers::postprocessorTestSolver::momentumPredictor()
{
}

void
Foam::solvers::postprocessorTestSolver::thermophysicalPredictor()
{
  // To set temperature for testing, internal energy must be set. The
  // thermo_.correct() call calculates Temperature.

  // Get e and Cv
  volScalarField & e = thermo_.he();
  const volScalarField & Cv = thermo_.Cv();

  // Set e to Cv*(xy + yz + xz)t which gives a non-uniform be first order value of wall heat flux at
  // all boundaries.
  dimensioned<Foam::scalar> t("t", T.dimensions(), mesh_.time().userTimeValue());
  e = Cv * t;

  thermo_.correct();
}

void
Foam::solvers::postprocessorTestSolver::pressureCorrector()
{
}

void
Foam::solvers::postprocessorTestSolver::postCorrector()
{
}

void
Foam::solvers::postprocessorTestSolver::postSolve()
{
}

// ************************************************************************* //
