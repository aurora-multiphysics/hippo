# Build custom OpenFOAM source files: this should be improved in future
all: build_foam_tests

MAKE=wmake
MAKEFLAGS += --no-print-directory

build_foam_tests:
	$(info Building Hippo's OpenFOAM test modules)
	@$(MAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/functionTestSolver/
	@$(MAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/laplacianTestSolver/
	@$(MAKE) -s -j $(MOOSE_JOBS) test/OpenFOAM/modules/odeTestSolver/
