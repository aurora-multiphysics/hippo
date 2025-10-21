# Build custom OpenFOAM source files: this should be improved in future
all: build_foam_tests

MAKE=wmake
build_foam_tests:
	$(info Building Hippo's OpenFOAM test modules)
	@$(MAKE) -j $(MOOSE_JOBS) test/OpenFOAM/modules/functionTestSolver/ 1>/dev/null
	@$(MAKE) -j $(MOOSE_JOBS) test/OpenFOAM/modules/laplacianTestSolver/ 1>/dev/null
	@$(MAKE) -j $(MOOSE_JOBS) test/OpenFOAM/modules/odeTestSolver/ 1>/dev/null
