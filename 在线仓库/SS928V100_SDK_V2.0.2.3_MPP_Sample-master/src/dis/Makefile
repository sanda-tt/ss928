
include ../Makefile.param

SMP_SRCS := sample_dis.c
ifeq ($(GYRO_DIS), y)
SMP_SRCS += sample_gyro_dis.c
SMP_SRCS += sample_fov2ldc.c
SMP_SRCS += sample_dis_two_pipe.c
endif
TARGET := sample_dis

TARGET_PATH := $(PWD)

OBJS  := $(SMP_SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)