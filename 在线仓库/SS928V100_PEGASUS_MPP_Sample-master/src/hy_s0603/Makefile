include ../Makefile.param

LIBPATH = ./lib
OBJPATH = $(TEMP_PATH)/$(subst $(SDK_PATH)/,,$(CURDIR))/obj

ARFLAGS = rcv
ARFLAGS_SO = $(LIBS_LD_CFLAGS)
ARFLAGS_SO += -shared -fPIC -o
CFLAGS  += -Wall -fPIC -D$(OT_FPGA)
CFLAGS  += -O2
CFLAGS  += $(LIBS_CFLAGS)

ifeq ($(ISP_INI_CONFIG), y)
CFLAGS += -D INIFILE_CONFIG_MODE
endif

ifeq ($(OT_GDB),OT_GDB)
CFLAGS += -g
endif

ifeq ($(CONFIG_JPEGEDCF), y)
     CFLAGS += -D ENABLE_JPEGEDCF
endif

CUR_INC := ./
INC := $(COMM_INC)
DEPEND_FILE := $(foreach file,$(subst -I, ,$(CUR_INC)), $(wildcard $(file)/*.h))

# COMPILE = $(CC) $(CFLAGS) $(DFLAGS) -lm

# $(CUR_INC)/%.o: ./%.c $(DEPEND_FILE)
# 	@[ -e $(LIBPATH) ] || mkdir -p $(LIBPATH)
# 	@$(COMPILE) -o $@ -c $< $(INC)

SRCS = $(wildcard ./*.c)
OBJS = $(SRCS:%.c=%.o)

# CFLAGS += -I$(REL_INC) -I$(CUR_INC)
# $(error "===[$(COMM_INC)]====")

LIB_NAME := libsns_hy_s0603
TARGETLIB := $(LIBPATH)/$(LIB_NAME).a
TARGETLIB_SO := $(LIBPATH)/$(LIB_NAME).so

all:$(TARGETLIB)
$(TARGETLIB):$(OBJS)
	@mkdir -p $(LIBPATH)
	@($(AR) $(ARFLAGS) $(TARGETLIB) $(OBJS))
	@($(CC) $(ARFLAGS_SO) $(TARGETLIB_SO) $(OBJS))
	@cp -f $(TARGETLIB) $(REL_LIB)/
	@cp -f $(TARGETLIB_SO) $(REL_LIB)/

clean:
	@$(RM) -rf $(TARGETLIB) $(OBJS)
	@$(RM) -rf $(LIBPATH)

