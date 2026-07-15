include ../Makefile.param

########### SRCS & INC ############
C_SRCS   := $(wildcard ../svp/common/sample_common_ive.c)
CPP_SRCS := $(wildcard $(PWD)/*.cpp)

TARGET := sample_opencv

CFLAGS += -I$(PWD)/../svp/common/
CFLAGS += -L$(3RDPARTY_LIBS_PATH)

MPI_LIBS += $(REL_LIB)/libss_ive.a
MPI_LIBS += -lopencv_world

TARGET_PATH := $(PWD)

C_OBJS := $(C_SRCS:%.c=%.o)
CPP_OBJS := $(CPP_SRCS:%.cpp=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(CPP_OBJS) $(C_OBJS)
	@rm -f $(COMM_OBJ)
