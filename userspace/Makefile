CC ?= aarch64-linux-gnu-gcc
CFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS += -I../kernel
TARGET := akdctl
SRCS := akdctl.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
