include ../Makefile.param

########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

TARGET := sample_hnr

TARGET_PATH := $(PWD)

CFLAGS += -DSAMPLE_HNR_SUPPORT
MPI_LIBS += $(REL_LIB)/libss_pqp.a
MPI_LIBS += $(REL_LIB)/libss_hnr.a

MPI_LIBS += $(REL_LIB)/libss_mcf.a
MPI_LIBS += $(REL_LIB)/libss_mcf_vi.a

OBJS  := $(SMP_SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

