/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2022-2023 OpenFOAM Foundation
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

Class
    Foam::solvers::bcTestSolver

Description
    Solver module for to test imposition of OpenFOAM boundary conditions using Moose input file
    syntax. Based on Solid solver

SourceFiles
    bcTestSolver.C

\*---------------------------------------------------------------------------*/

#pragma once

#include "baseTestSolver.H"
#include "solidThermophysicalTransportModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class bcTestSolver Declaration
\*---------------------------------------------------------------------------*/

class bcTestSolver : public baseTestSolver
{

protected:
  // Control parameters
  scalar maxDeltaT_;

  // Thermophysical properties
  autoPtr<solidThermo> thermoPtr_;

  solidThermo & thermo_;

  volScalarField T_;

  autoPtr<solidThermophysicalTransportModel> thermophysicalTransport;

public:
  // Public Data
  // reference to thermophysical properties
  const solidThermo & thermo;

  //- Reference to the temperature field
  const volScalarField & T;

  //- Runtime type information
  TypeName("bcTest");

  // Constructors

  //- Construct from region mesh
  bcTestSolver(fvMesh & mesh, autoPtr<solidThermo> thermoPtr);

  bcTestSolver(fvMesh & mesh);

  //- Disallow default bitwise copy construction
  bcTestSolver(const bcTestSolver &) = delete;

  virtual void preSolve();

  //- Construct and solve the energy equation,
  //  convert to temperature
  //  and update thermophysical and transport properties
  virtual void thermophysicalPredictor();
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //
