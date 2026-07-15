include $(PWD)/../Makefile.param

CFLAGS += -I$(PWD)/include -O3
CFLAGS := $(filter-out -I$(PWD)/../common, $(CFLAGS))
CFLAGS += -I$(REL_INC)/svp_npu

SMP_SRCS := $(wildcard ./*.c)
SMP_SRCS += $(wildcard ./sample_svp_npu/*.c)
TARGET := sample_ovd_sot
TARGET_PATH := $(PWD)

MPI_LIBS += $(REL_LIB)/libiaa_alg.a $(REL_LIB)/libobject_track.a
MPI_LIBS += $(REL_LIB)/libss_ive.a $(REL_LIB)/svp_npu/libsvp_acl.a $(REL_LIB)/svp_npu/libprotobuf-c.a $(REL_LIB)/libss_mpi.a $(REL_LIB)/libss_dpu_rect.a $(REL_LIB)/libss_pqp.a

OBJS  := $(SMP_SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)