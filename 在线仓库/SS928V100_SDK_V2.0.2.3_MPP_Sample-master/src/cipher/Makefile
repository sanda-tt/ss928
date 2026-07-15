include ../Makefile.param


########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

MPI_LIBS += $(REL_LIB)/libss_cipher.a
MPI_LIBS += $(REL_LIB)/libss_klad.a

TARGET := sample_cipher

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