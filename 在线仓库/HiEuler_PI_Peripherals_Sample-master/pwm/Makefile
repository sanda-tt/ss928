include ../Makefile.param

SMP_SRCS := $(wildcard *.c)
SMP_OBJS := $(SMP_SRCS:%.c=%.o)

TARGET := hi_pwm_sample

all: $(TARGET)

$(TARGET): $(SMP_OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SMP_OBJS) $(TARGET)

.PHONY: all clean
