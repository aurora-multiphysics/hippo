#pragma once
// Need to play some games to prevent name clashes
// hence the undef and to define namespaceFoam to
// prevnet a using namespace Foam; in header (sigh)
#define namespaceFoam
#include "fvCFD.H"
#undef NotImplemented
