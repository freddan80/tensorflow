ifneq (,$(filter $(TARGET_ARCH), cortex-m3 cortex-m4 cortex-m7))
    # Let's say the depthwise_conv is implemented for this architecture =>
    # overrride the reference kernel
    CXXFLAGS += -DARM_MATH_CM4
    CCFLAGS += -DARM_MATH_CM4
endif

ifneq ($(filter cmsis-nn,$(ALL_TAGS)),)
    # Setup CMSIS-NN lib and add required header files to microlite lib INCLUDE
    KERNEL_LIB_PATH = $(MAKEFILE_DIR)/downloads/cmsis/
    MICROLITE_CC_SRCS += $(shell find $(KERNEL_LIB_PATH)/CMSIS/NN/Source/ -name *.c)
    INCLUDES += -I$(KERNEL_LIB_PATH)/CMSIS/Core/Include \
                -I$(KERNEL_LIB_PATH)/CMSIS/NN/Include \
                -I$(KERNEL_LIB_PATH)/CMSIS/DSP/Include
endif
