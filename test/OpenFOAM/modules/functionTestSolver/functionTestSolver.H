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
    Foam::solvers::functionTestSolver

Description
    A test solver for fixed-point iteration that checks that fields are restored
    correctly between fixed-points and that consistant behaviour is observed
    for time derivataives.

SourceFiles
    functionTestSolver.C

\*---------------------------------------------------------------------------*/

#ifndef FunctionTestSolver_H
#define FunctionTestSolver_H

#include "baseTestSolver.H"
#include "solidThermophysicalTransportModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class functionTestSolver Declaration
\*---------------------------------------------------------------------------*/

class functionTestSolver : public baseTestSolver
{

protected:
  // Control parameters
  scalar maxDeltaT_;

  // Thermophysical properties

  volScalarField T_;

  volScalarField dTdt_;

  volScalarField kappa_;

public:
  // Public Data

  //- Reference to the temperature field
  const volScalarField & T;

  const volScalarField & dTdt;

  const volScalarField & kappa;

  //- Runtime type information
  TypeName("functionTestSolver");

  // Constructors

  //- Construct from region mesh
  functionTestSolver(fvMesh & mesh);

  //- Disallow default bitwise copy construction
  functionTestSolver(const functionTestSolver &) = delete;

  // Member Functions

  //- Called at the start of the time-step, before the PIMPLE loop
  virtual void preSolve();

  //- Called at the start of the PIMPLE loop to move the mesh
  virtual void moveMesh();

  //- Construct and solve the energy equation,
  //  convert to temperature
  //  and update thermophysical and transport properties
  virtual void thermophysicalPredictor();

  // Member Operators

  //- Disallow default bitwise assignment
  void operator=(const functionTestSolver &) = delete;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
