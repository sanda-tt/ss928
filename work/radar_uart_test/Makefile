CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
TARGET := radar_uart_dump

all: $(TARGET)

$(TARGET): radar_uart_dump.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
