
include ../Makefile.param

INC_FLAGS +=-I./include
SMP_SRCS += $(wildcard *.c)
SMP_SRCS += $(wildcard ./src/*.c)
TARGET := $(SMP_SRCS:%.c=%)

TARGET_PATH := $(PWD)
INIPARSER_LIB := $(3RDPARTY_LIBS_PATH)/libiniparser.a
CFLAGS += $(INC_FLAGS)

OBJS  := $(SMP_SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)