include ../Makefile.param

CFLAGS += -I$(PWD)/sample
CFLAGS += -I$(PWD)/encapsulation/include

SMP_SRCS := $(wildcard *.c)
SMP_SRCS += $(wildcard ./sample/*.c)
SMP_SRCS += $(wildcard ./encapsulation/src/*.c)
SMP_SRCS += $(wildcard $(PWD)/../common/*.c)
TARGET := sample_dsp_main

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