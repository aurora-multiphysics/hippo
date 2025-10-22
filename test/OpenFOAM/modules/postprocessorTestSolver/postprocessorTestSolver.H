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
    Foam::solvers::postprocessorTestSolver

Description
    Solver module for to test transfers between OpenFOAM and MOOSE. Based on Solid
    solver

SourceFiles
    postprocessorTestSolver.C

\*---------------------------------------------------------------------------*/

#pragma once

#include "fluid.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solvers
{

/*---------------------------------------------------------------------------*\
                          Class postprocessorTestSolver Declaration
\*---------------------------------------------------------------------------*/

class postprocessorTestSolver : public fluid
{

public:
  //- Runtime type information
  TypeName("postprocessorTestSolver");

  // Constructors

  postprocessorTestSolver(fvMesh & mesh);

  //- Disallow default bitwise copy construction
  postprocessorTestSolver(const postprocessorTestSolver &) = delete;

  //- Destructor
  virtual ~postprocessorTestSolver();

  // Member Functions

  //- Called at the start of the time-step, before the PIMPLE loop
  virtual void preSolve();

  //- Called at the start of the PIMPLE loop to move the mesh
  virtual void moveMesh();

  //- Corrections that follow mesh motion
  virtual void motionCorrector();

  //- Called at the beginning of the PIMPLE loop
  virtual void prePredictor();

  //- Construct and optionally solve the momentum equation
  virtual void momentumPredictor();

  //- Construct and solve the energy equation,
  //  convert to temperature
  //  and update thermophysical and transport properties
  virtual void thermophysicalPredictor();

  //- Construct and solve the pressure equation in the PISO loop
  virtual void pressureCorrector();

  //- Correct the thermophysical transport modelling
  virtual void postCorrector();

  //- Called after the PIMPLE loop at the end of the time-step
  virtual void postSolve();

  // Member Operators

  //- Disallow default bitwise assignment
  void operator=(const postprocessorTestSolver &) = delete;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solvers
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //
