# Build custom OpenFOAM source files: this should be improved in future
all: build_foam_tests

build_foam_tests:
	$(info Building Hippo's OpenFOAM test modules)
	wmake test/OpenFOAM/modules/transferTestSolver/
