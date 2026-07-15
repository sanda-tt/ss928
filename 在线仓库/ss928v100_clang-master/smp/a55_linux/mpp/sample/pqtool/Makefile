SDK_DIR := /home/xxx/ss928v100_clang

-include $(shell pwd)/Makefile.param

SRC_DIR := $(shell pwd)
CONTROL_LIB_DIR := $(shell pwd)/../release
COMMON_INCLUDE = $(SDK_DIR)/smp/a55_linux/mpp/sample/common

TARGET := test_pqt

LIBS :=
LIBS+= $(SDK_DIR)/smp/a55_linux/mpp/out/lib/lib_pqstream.a
LIBS+= $(SDK_DIR)/smp/a55_linux/mpp/out/lib/lib_pqcontrol.a

LIBS+= -L$(SDK_DIR)/smp/a55_linux/mpp/out/lib
LIBS+= -lss_isp -lot_isp -lss_mpi -lss_upvqe -lss_dnvqe -lss_voice_engine -lsecurec -lss_drc -lss_dehaze -lss_ldci -lss_calcflicker -lss_tde -lss_acs -lss_ir_auto -lss_crb -lss_bnr\
       -lss_motionfusion -lss_snap -lss_ae -lss_awb -lss_extend_stats
LIBS+= -lss_pqp -lss_hnr -lss_dpu_match -lss_dpu_rect
LIBS+= -lss_hdmi

LIBS += $(SDK_DIR)/smp/a55_linux/mpp/out/lib/svp_npu/libsvp_acl.so
LIBS += -L$(SDK_DIR)/smp/a55_linux/mpp/out/lib/svp_npu -lprotobuf-c

MPP_DIR = $(SDK_DIR)/smp/a55_linux/mpp/out
SDK_LIB_PATH := -L$(MPP_DIR)/lib -L$(MPP_DIR)/lib/extdrv
SDK_LIB := $(SDK_LIB_PATH)
SDK_LIB += -lot_osal
SDK_LIB += -lss_mpi
SDK_LIB += -lot_base
SDK_LIB += -lot_sys
#SDK_LIB += -lot_vb
SDK_LIB += -lot_chnl
SDK_LIB += -lot_rgn
SDK_LIB += -lot_vpp
SDK_LIB += -lot_vo
SDK_LIB += -lot_vgs
SDK_LIB += -lot_vpss
SDK_LIB += -lot_vdec
SDK_LIB += -lot_h265e
SDK_LIB += -lot_h264e
SDK_LIB += -lot_jpege
SDK_LIB += -lot_venc
#SDK_LIB += -lot_venc_svr
SDK_LIB += -lot_vedu
SDK_LIB += -lot_vi
SDK_LIB += -lot_mpi_isp
SDK_LIB += -lot_rc
SDK_LIB += -lot_jpegd
SDK_LIB += -lot_h265e
SDK_LIB += -lss_hdmi
SDK_LIB += -lot_vfmw
SDK_LIB += -lot_gdc
SDK_LIB += -lot_dis
SDK_LIB += -lot_irq
SDK_LIB += -lss_dpu_match
SDK_LIB += -lss_dpu_rect
SDK_LIB += -lss_mau
SDK_LIB += -lss_ive
#SDK_LIB += -lot_avs
# SDK_LIB += -lot_mcf
# SDK_LIB += -lot_mcf_vi
SDK_LIB += -lot_sensor_i2c
SDK_LIB += -lot_sensor_spi
#SDK_LIB += -ltde

SDK_LIB += -lot_aio
SDK_LIB += -lot_ai
SDK_LIB += -lot_ao
SDK_LIB += -lot_aenc
SDK_LIB += -lot_adec
SDK_LIB += -lot_acodec
SDK_LIB += -lot_pwm
SDK_LIB += -lot_piris

LIBS+= $(SDK_LIB)


SDK_INCLUDE = $(SDK_DIR)/smp/a55_linux/mpp/out/include

SRC := $(SRC_DIR)/test_main.c 
SRC += $(COMMON_INCLUDE)/sdk_init.c
SRC += $(COMMON_INCLUDE)/sdk_exit.c
OBJ := $(SRC:%.c=%.o)

CFLAGS := -fPIC -Wall -I$(COMMON_INCLUDE) -I$(SDK_INCLUDE) -lm -ldl -lpthread -lstdc++ -mcpu=cortex-a53

.PHONY: all $(TARGET) clean

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	
clean:
	@rm -f $(TARGET)
	@rm -f ../common/*.o
	@rm -f *.o
