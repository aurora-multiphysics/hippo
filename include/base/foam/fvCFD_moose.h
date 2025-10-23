#pragma once
// This file is a workaround for a name clash between OpenFOAM and libMesh,
// which both define the symbol 'NotImplemented'. If you see some cryptic
// build errors in libMesh, then this is likely the problem.
// The solution here is to include what we need from OpenFOAM and then undef
// NotImplemented, which will be redefined by libMesh later. Source files can
// include this file instead of directly including OpenFOAM headers.
// Not a huge amount of science went into what's included here and what's
// included in source files. It was pretty much a case of 'move things until it
// works'.

#define namespaceFoam
// Foam2MooseMeshGen.h
#include <distributionMapBase.H>
#include <fvMesh.H>

// Foam2MooseMeshGen.C
#include <IOobject.H>
#include <PrimitivePatch.H>
#include <error.H>
#include <fvMesh.H>
#include <polyPatch.H>

// FoamSolver.h
#include <scalar.H>
#include <solver.H>

// FoamRuntime.h
#include <Time.H>

// FoamRuntime.C
#include <Pstream/mpi/PstreamGlobals.H>
#include <argList.H>
#include <fvMesh.H>

// FoamDataStore.h
#include <error.H>
#include <surfaceFields.H>
#include <volFields.H>
#include <pointFields.H>
#include <typeInfo.H>
#include "symmTensorField.H"
#include "DimensionedField.H"
#include "uniformDimensionedFields.H"
#include "ddtScheme.H"

#undef NotImplemented
