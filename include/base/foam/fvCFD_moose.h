#pragma once
// Need to play some games to prevent name clashes
// hence the undef and to define namespaceFoam to
// prevnet a using namespace Foam; in header (sigh)

#define namespaceFoam
// Foam2MooseMeshGen.C
#include <error.H>
#include <fvMesh.H>
#include <IOobject.H>
#include <polyPatch.H>

// FoamInterfaceImpl.H
#include <argList.H>

// FoamSolver.H
#include <solver.H>

#undef NotImplemented
