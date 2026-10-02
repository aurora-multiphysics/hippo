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
    Foam::solvers::mappedInletTestSolver

Description
    Solver module for to test mapped inlet implementation method

SourceFiles
    mappedInletTestSolver.C

\*---------------------------------------------------------------------------*/

#pragma once

#include "baseTestSolver.H"
#include "solidThermophysicalTransportModel.H"
#include "volFieldsFwd.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class mappedInletTestSolver Declaration
\*---------------------------------------------------------------------------*/

class mappedInletTestSolver : public baseTestSolver
{

protected:
  // Control parameters
  scalar maxDeltaT_;

  // Thermophysical properties
  autoPtr<solidThermo> thermoPtr_;

  solidThermo & thermo_;

  volScalarField T_;

  volVectorField U_;

  autoPtr<solidThermophysicalTransportModel> thermophysicalTransport;

public:
  // Public Data
  // reference to thermophysical properties
  solidThermo & thermo;

  //- Reference to the temperature field
  const volScalarField & T;

  //- Reference velocity field
  const volVectorField & U;

  //- Runtime type information
  TypeName("mappedInletTest");

  // Constructors

  //- Construct from region mesh
  mappedInletTestSolver(fvMesh & mesh, autoPtr<solidThermo> thermoPtr);

  mappedInletTestSolver(fvMesh & mesh);

  //- Disallow default bitwise copy construction
  mappedInletTestSolver(const mappedInletTestSolver &) = delete;

  // Member Functions

  //- Called at the start of the time-step, before the PIMPLE loop
  virtual void preSolve();

  //- Construct and optionally solve the momentum equation
  virtual void momentumPredictor();

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
