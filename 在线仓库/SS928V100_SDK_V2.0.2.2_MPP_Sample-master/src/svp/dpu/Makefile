include ../Makefile.param

SMP_SRCS := $(wildcard *.c)
SMP_SRCS += $(wildcard ./sample_svp_dpu/*.c)
SMP_SRCS += $(wildcard ./common/*.c)

CFLAGS += -I$(PWD)/common/
CFLAGS += -I$(PWD)/sample_svp_dpu/
TARGET := sample_dpu_main
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

