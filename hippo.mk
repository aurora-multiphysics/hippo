
FOAM_ROOT_DIR := $(shell echo $(WM_PROJECT_DIR)/platforms/$(WM_OPTIONS))

FOAM_LIB_DIR := $(FOAM_ROOT_DIR)/lib

FOAM_INCLUDE_ROOT := $(shell echo $(WM_PROJECT_DIR)/src)
ADDITIONAL_LIBS := -L$(FOAM_LIB_DIR) \
    -lfiniteVolume \
    -lfvModels \
    -lfvConstraints \
    -lmeshTools \
    $(ADDITIONAL_LIBS)

ADDITIONAL_INCLUDES := -I$(FOAM_INCLUDE_ROOT)/finiteVolume/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/conversion/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/meshTools/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/OpenFOAM/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/MomentumTransportModels/momentumTransportModels/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/ThermophysicalTransportModels/fluidThermo/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/ThermophysicalTransportModels/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/MomentumTransportModels/compressible/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/physicalProperties/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/thermophysicalModels/basic/lnInclude \
    -I$(FOAM_INCLUDE_ROOT)/OSspecific/POSIX/lnInclude \
    $(ADDITIONAL_INCLUDES)


ADDITIONAL_CPPFLAGS += $(shell echo -D$(WM_ARCH) \
                                    -DWM_ARCH_OPTION=$(WM_ARCH_OPTION)\
                                    -DWM_LABEL_SIZE=$(WM_LABEL_SIZE)\
                                    -DWM_$(WM_PRECISION_OPTION) \
                                    -DNoRepository -ftemplate-depth-100 \
                                    -Xlinker --add-needed -Xlinker --no-as-needed -fuse-ld=bfd )




$(info Additional libraries: $(ADDITIONAL_LIBS))
$(info Additional includes: $(ADDITIONAL_INCLUDES))
$(info Additional flags: $(ADDITIONAL_CPPFLAGS))
