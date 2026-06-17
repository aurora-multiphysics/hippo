# Build custom OpenFOAM source files: this should be improved in future
all: build_foam_tests

WMAKE ?= wmake
MAKEFLAGS += --no-print-directory

build_foam_tests:
	$(info Building Hippo's OpenFOAM test modules)
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/transferTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/bcTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/functionTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/laplacianTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/odeTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/postprocessorTestSolver/
	+@$(WMAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/mappedInletTestSolver/
