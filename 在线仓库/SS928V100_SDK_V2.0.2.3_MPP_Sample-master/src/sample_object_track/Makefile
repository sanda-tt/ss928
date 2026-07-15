include ../Makefile.param

SMP_SRCS := $(PWD)/src/main.c $(PWD)/src/config.c
TARGET := sample_object_track

TARGET_PATH := $(PWD)
CFLAGS += -I$(TRACK_ROOT)/include -I$(IAA_ROOT)/include -I$(PWD)/src
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