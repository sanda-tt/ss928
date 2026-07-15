include ../Makefile.param

########### SRCS & INC ############
SMP_SRCS_C := $(wildcard ../svp/common/sample_common_ive.c)
SMP_SRCS_CPP := $(wildcard $(PWD)/*.cpp)

CFLAGS += -fPIC

CXXFLAGS += $(CFLAGS)
CXXFLAGS += -fPIC -std=c++11

CFLAGS += -I$(PWD)/../svp/common/
CXXFLAGS += -I$(PWD)/include
CXXFLAGS += -I$(PWD)/include/python3.10

MPI_LIBS += $(REL_LIB)/libss_ive.a

OBJS_C := $(SMP_SRCS_C:%.c=%.o)
OBJS_CPP := $(SMP_SRCS_CPP:%.cpp=%.o)

OBJS := $(OBJS_C) $(OBJS_CPP)

TARGET := sample_pymodule.so
TARGET_PATH := $(PWD)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(OBJS) $(SUNSHINE_COMM_OBJ)
	$(CXX) $(CXXFLAGS) -shared -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS)  $(INIPARSER_LIB) -Wl,--end-group

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS) 
	@rm -f $(SUNSHINE_COMM_OBJ)
