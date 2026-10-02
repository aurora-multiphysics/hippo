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

#include "dimensionedScalar.H"
#include "dimensionedVector.H"
#include "fvMesh.H"
#include "mappedInletTestSolver.H"
#include "fvMeshMover.H"
#include "addToRunTimeSelectionTable.H"
#include "fvConstraints.H"
#include "fvmLaplacian.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{
defineTypeNameAndDebug(mappedInletTestSolver, 0);
addToRunTimeSelectionTable(solver, mappedInletTestSolver, fvMesh);
}
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
// Solver based on solid.C module
Foam::solvers::mappedInletTestSolver::mappedInletTestSolver(fvMesh & mesh,
                                                            autoPtr<solidThermo> thermoPtr)
  : baseTestSolver(mesh),

    thermoPtr_(thermoPtr),
    thermo_(thermoPtr_()),

    T_(IOobject("T", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE), mesh),

    U_(IOobject("U", mesh.time().name(), mesh, IOobject::NO_READ, IOobject::AUTO_WRITE), mesh),

    thermophysicalTransport(solidThermophysicalTransportModel::New(thermo_)),
    thermo(thermo_),
    T(T_),
    U(U_)
{
  thermo.validate("solid", "h", "e");
}

Foam::solvers::mappedInletTestSolver::mappedInletTestSolver(fvMesh & mesh)
  : mappedInletTestSolver(mesh, solidThermo::New(mesh))
{
  // Read the controls
  read();
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void
Foam::solvers::mappedInletTestSolver::preSolve()
{
  fvModels().preUpdateMesh();

  // Update the mesh for topology change, mesh to mesh mapping
  mesh_.update();
}
void
Foam::solvers::mappedInletTestSolver::momentumPredictor()
{
  auto & coords = mesh.C().primitiveField();
  auto & U_field = U_.primitiveFieldRef();
  auto time = mesh.time().userTimeValue();

  auto x = coords.component(0)();
  auto y = coords.component(1)();
  auto z = coords.component(2)();

  U_field.replace(0, x + y + z + time);
  U_field.replace(1, x - y + z + time);
  U_field.replace(2, x + y - z + time);
}

void
Foam::solvers::mappedInletTestSolver::thermophysicalPredictor()
{
  volScalarField & e = thermo.he();
  auto & coords = mesh.C().primitiveField();
  e.primitiveFieldRef() = mag(coords) + mesh.time().userTimeValue();
  thermo_.correct();
}

// ************************************************************************* //
