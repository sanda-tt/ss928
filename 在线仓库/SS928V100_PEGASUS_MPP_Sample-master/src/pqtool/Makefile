include ../Makefile.param
SDK_DIR := $(SDK_PATH)

SRC_DIR := $(shell pwd)
CONTROL_LIB_DIR := $(shell pwd)/../release
COMMON_INCLUDE = $(COMMON_DIR)

TARGET := test_pqt

LIBS :=
LIBS+= $(REL_LIB)/lib_pqstream.a
LIBS+= $(REL_LIB)/lib_pqcontrol.a

LIBS+= -L$(REL_LIB)
LIBS+= -lss_isp -lot_isp -lss_mpi -lss_upvqe -lss_dnvqe -lss_voice_engine -lsecurec -lss_drc -lss_dehaze -lss_ldci -lss_calcflicker -lss_tde -lss_acs -lss_ir_auto -lss_crb -lss_bnr\
       -lss_motionfusion -lss_snap -lss_ae -lss_awb -lss_extend_stats
LIBS+= -lss_pqp -lss_hnr -lss_dpu_match -lss_dpu_rect
LIBS+= -lss_hdmi
LIBS += $(REL_LIB)/svp_npu/libsvp_acl.a
LIBS += -L$(REL_LIB)/svp_npu -lprotobuf-c

LIBS+= $(SDK_LIB)


SDK_INCLUDE = $(REL_INC)

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
