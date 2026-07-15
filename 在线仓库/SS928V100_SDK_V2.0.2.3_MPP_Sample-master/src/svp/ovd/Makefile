include $(PWD)/../Makefile.param

CFLAGS += -I$(PWD)/include -O3
CFLAGS := $(filter-out -I$(PWD)/../common, $(CFLAGS))
CFLAGS += -I$(REL_INC)/svp_npu

SMP_SRCS := $(wildcard ./*.c)
SMP_SRCS += $(wildcard ./sample_svp_npu/*.c)
TARGET := sample_svp_npu_main
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