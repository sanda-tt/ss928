include $(PWD)/../Makefile.param

########### SRCS & INC ############
SMP_SRCS_C := $(wildcard $(PWD)/*.c)
SMP_SRCS_C += $(wildcard $(PWD)/sample_svp_npu/*.c)
SMP_SRCS_C += $(wildcard $(PWD)/../common/*.c)
SMP_SRCS_CPP := $(wildcard $(PWD)/yolo/*.cpp)

CFLAGS += -I$(PWD)/include
CFLAGS += -I$(PWD)/yolo
CFLAGS += -I$(SAMPLE_DIR)/include/3rdparty/ncnn
CFLAGS += -L$(REL_LIB)/npu/stub -lascendcl

CXXFLAGS += $(CFLAGS)
CXXFLAGS += -O3 -fopenmp
CXXFLAGS += -DUSE_NCNN_SIMPLEOCV

MPI_LIBS += $(3RDPARTY_LIBS_PATH)/libncnn.a

OBJS_C := $(SMP_SRCS_C:%.c=%.o)
OBJS_CPP := $(SMP_SRCS_CPP:%.cpp=%.o)

OBJS := $(OBJS_C) $(OBJS_CPP)

TARGET := sample_yolov8

TARGET_PATH := $(PWD)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS)  $(AUDIO_LIBS) -Wl,--end-group

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)
