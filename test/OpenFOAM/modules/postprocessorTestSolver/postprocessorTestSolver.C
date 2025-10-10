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
#include "dimensionedScalar.H"
#include "dimensionedVector.H"
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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
// Solver based on solid.C module
Foam::solvers::postprocessorTestSolver::postprocessorTestSolver(fvMesh & mesh) : fluid(mesh) {}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::solvers::postprocessorTestSolver::~postprocessorTestSolver() {}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

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
  volScalarField & h = thermo_.he();
  const volScalarField & Cp = thermo_.Cp();

  // Set e to Cv*(xy + yz + xz)t which gives a non-uniform be first order value of wall heat flux at
  // all boundaries.
  dimensioned<Foam::scalar> t("t", thermo_.T().dimensions(), mesh_.time().userTimeValue());
  h = Cp * t;

  U_ = dimensionedVector(dimVelocity, {1., 1., 1.});

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
