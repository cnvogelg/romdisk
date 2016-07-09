
DEBUG=1

CC=vc
CFLAGS=-c99 -g
LDFLAGS=-g -nostdlib

SRCS=device.c boot.c disk.c
HDRS=device.h boot.h disk.h debug.h

ifeq "$(DEBUG)" "1"
CFLAGS+=-DDEBUG=1
SRCS+=debug.c
endif

OBJS=$(patsubst %.c,%.o,$(SRCS))

all: ext.rom

ext.rom: romdisk.device
	romtool -v build -o $@ -t ext $< test.hdf

romdisk.device: $(OBJS)
	vc $^ -o $@ $(LDFLAGS)

$(SRCS): $(HDRS)

clean:
	rm -f *.o
	rm -f romdisk.device
