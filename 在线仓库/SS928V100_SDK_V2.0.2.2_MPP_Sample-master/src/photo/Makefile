include ../Makefile.param

########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

MPI_LIBS += $(REL_LIB)/libss_snap.a
MPI_LIBS += $(REL_LIB)/libss_photo.a
MPI_LIBS += $(REL_LIB)/libss_dsp.a
MPI_LIBS += $(REL_LIB)/libhdr_ap.a
MPI_LIBS += $(REL_LIB)/libmfnr_ap.a
MPI_LIBS += $(REL_LIB)/libsfnr_ap.a
MPI_LIBS += $(REL_LIB)/libdetail_ap.a

TARGET := sample_photo

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
