#include $(SCRIPTS_DIR)/mpp/sample/Makefile.param
include ../Makefile.param

SMP_SRCS := $(wildcard *.c)
TARGET := $(SMP_SRCS:%.c=%)

MPI_LIBS += $(REL_LIB)/libss_tde.a

#TARGET_PATH := $(OUTPUT_DIR)/ss928_fb_tool
TARGET_PATH := $(PWD)

CFLAGS += -DCONFIG_SUPPORT_SAMPLE_ROTATION

#include $(SCRIPTS_DIR)/mpp/sample/$(ARM_ARCH)_$(OSTYPE).mak
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
