include ../Makefile.param

SMP_SRCS := $(wildcard *.c)
TARGET := sample_photo

MPI_LIBS += $(REL_LIB)/libss_snap.a
MPI_LIBS += $(REL_LIB)/libss_photo.a
MPI_LIBS += $(REL_LIB)/libss_dsp.a
MPI_LIBS += $(REL_LIB)/libhdr_ap.a
MPI_LIBS += $(REL_LIB)/libmfnr_ap.a
MPI_LIBS += $(REL_LIB)/libsfnr_ap.a
MPI_LIBS += $(REL_LIB)/libdetail_ap.a

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