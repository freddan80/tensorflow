ifeq ($(KERNEL_LIB), cmsis)
  KERNEL_ROOT := tensorflow/lite/experimental/micro/kernels

  # Below are example of how the depthwise_conv kernel is replaced with an
  # architecture.
  ifneq (,$(filter $(TARGET_ARCH), cortex-m0 cortex-m1))
    # Let's say the depthwise_conv isn't implemented for this architecture =>
    # reference kernel will be used
  else ifneq (,$(filter $(TARGET_ARCH), cortex-m3 cortex-m4 cortex-m7))
    # Let's say the depthwise_conv is implemented for this architecture =>
    # overrride the reference kernel
    CXXFLAGS += -DARM_MATH_CM4
    CCFLAGS += -DARM_MATH_CM4
    REPLACE_KERNELS := $(KERNEL_ROOT)/depthwise_conv.cc
    KERNELS_OPTIMIZED := $(KERNEL_ROOT)/cmsis-nn/depthwise_conv.cc
  else ifneq (,$(filter $(TARGET_ARCH), cortex-m23 cortex-m33))
    # Add more optimized kernels like this:
    REPLACE_KERNELS := $(KERNEL_ROOT)/depthwise_conv.cc
    REPLACE_KERNELS += $(KERNEL_ROOT)/softmax.cc
    KERNELS_OPTIMIZED := $(KERNEL_ROOT)/cmsis-nn/depthwise_conv.cc
    KERNELS_OPTIMIZED += $(KERNEL_ROOT)/cmsis-nn/softmax.cc
  else
    # x86_64 etc. only used for testing purposes
    REPLACE_KERNELS := $(KERNEL_ROOT)/depthwise_conv.cc
    KERNELS_OPTIMIZED := $(KERNEL_ROOT)/cmsis-nn/depthwise_conv.cc
  endif

  # Check if optimized kernals exist
  ifneq ($(KERNELS_OPTIMIZED),)
    # Check that dependency variables are defined
    ifeq ($(MICROLITE_CC_SRCS),)
      $(info Source files missing, MICROLITE_CC_SRCS: $(MICROLITE_CC_SRCS))
    endif
    ifeq ($(MAKEFILE_DIR),)
      $(info Makefile path not defined, MAKEFILE_DIR: $(MAKEFILE_DIR))
    endif
    ifeq ($(LIBDIR),)
      $(info Lib path not defined, LIBDIR: $(LIBDIR))
    endif
    ifeq ($(OBJDIR),)
      $(info Obj path not defined, OBJDIR: $(OBJDIR))
    endif

    $(info CMSIS-NN target architecture: $(TARGET_ARCH))
    $(info CMSIS-NN optimized kernels: $(KERNELS_OPTIMIZED))

    # Replace ref kernel with CMSIS optimized kernel
    MICROLITE_CC_SRCS := $(filter-out $(REPLACE_KERNELS), $(MICROLITE_CC_SRCS))
    MICROLITE_CC_SRCS += $(KERNELS_OPTIMIZED)
    $(foreach var, $(MICROLITE_CC_SRCS),$(info $(var)))

    # Setup CMSIS-NN lib and add required header files to microlite lib INCLUDE
    KERNEL_LIB_PATH = $(MAKEFILE_DIR)/downloads/cmsis/
    KERNEL_LIB_NAME := lib-cmsis.a
    SOURCE_CMSIS := $(shell find $(KERNEL_LIB_PATH)/CMSIS/NN/Source/ -name *.c)
    INCLUDES += -I$(KERNEL_LIB_PATH)/CMSIS/Core/Include \
                -I$(KERNEL_LIB_PATH)/CMSIS/NN/Include \
                -I$(KERNEL_LIB_PATH)/CMSIS/DSP/Include
    KERNEL_LIBS_PATH += $(LIBDIR)$(KERNEL_LIB_NAME)
    KERNEL_LIB_OBJS := $(addprefix $(OBJDIR), $(patsubst %.c,%.o,$(SOURCE_CMSIS)))

# Recipe for the CMSIS-NN lib
$(KERNEL_LIBS_PATH): $(KERNEL_LIB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $(KERNEL_LIBS_PATH) $(KERNEL_LIB_OBJS)

  else
    $(info No optimized kernel available for this architecture: $(TARGET_ARCH))
  endif
endif
