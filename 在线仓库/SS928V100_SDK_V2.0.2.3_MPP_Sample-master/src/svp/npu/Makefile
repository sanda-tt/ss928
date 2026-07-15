include $(PWD)/../Makefile.param

CFLAGS += -I$(PWD)/include
CFLAGS += -O3
CFLAGS += -L$(REL_LIB)/npu/stub -lascendcl
INC_FLAGS += -I$(REL_INC)/npu

SMP_SRCS := $(wildcard ./*.c)
SMP_SRCS += $(wildcard ./sample_svp_npu/*.c)
SMP_SRCS += $(wildcard $(PWD)/../common/*.c)
TARGET := sample_npu_main
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