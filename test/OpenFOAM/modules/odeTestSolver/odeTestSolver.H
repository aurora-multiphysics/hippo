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
    Foam::solvers::odeTestSolver

Description
    A test solver for fixed-point iteration that checks that checks
    that different time schemes work correctly with fixed-point
    iteration

SourceFiles
    odeTestSolver.C

\*---------------------------------------------------------------------------*/

#ifndef solid_H
#define solid_H

#include "baseTestSolver.H"
#include "solidThermophysicalTransportModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class odeTestSolver Declaration
\*---------------------------------------------------------------------------*/

class odeTestSolver : public baseTestSolver
{

protected:
  // Control parameters
  scalar maxDeltaT_;

  // Thermophysical properties

  volScalarField T_;

  volScalarField kappa_;

  // Protected Member Functions

public:
  // Public Data

  //- Reference to the temperature field
  const volScalarField & T;

  const volScalarField & kappa;

  //- Runtime type information
  TypeName("odeTest");

  // Constructors

  //- Construct from region mesh
  odeTestSolver(fvMesh & mesh);

  //- Disallow default bitwise copy construction
  odeTestSolver(const odeTestSolver &) = delete;

  // Member Functions

  //- Return the current maximum time-step for stable solution
  virtual scalar maxDeltaT() const;

  //- Called at the start of the time-step, before the PIMPLE loop
  virtual void preSolve();

  //- Construct and solve the energy equation,
  //  convert to temperature
  //  and update thermophysical and transport properties
  virtual void thermophysicalPredictor();

  // Member Operators

  //- Disallow default bitwise assignment
  void operator=(const odeTestSolver &) = delete;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
