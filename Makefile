
DEBUG=1

CC=vc
CFLAGS=-c99

SRCS=device.c boot.c
HDRS=device.h boot.h debug.h

ifeq "$(DEBUG)" "1"
CFLAGS+=-DDEBUG=1
SRCS+=debug.c
endif

OBJS=$(patsubst %.c,%.o,$(SRCS))

all: ext.rom

ext.rom: romdisk.device
	romtool build -o $@ -t ext $<

romdisk.device: $(OBJS)
	vc $^ -o $@ -nostdlib

$(SRCS): $(HDRS)

clean:
	rm -f *.o
	rm -f romdisk.device
