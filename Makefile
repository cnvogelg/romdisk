
TC_DIR=/opt/m68k-amigaos

DEBUG=1

CC=vc
CFLAGS=-c99 -g -sc
LDFLAGS=-g -nostdlib
AS=vasmm68k_mot -Fhunk -I$(TC_DIR)/os-include

SRCS=device.c boot.c disk.c
ASRCS=diag.s
HDRS=device.h boot.h disk.h debug.h

ifeq "$(DEBUG)" "1"
CFLAGS+=-DDEBUG=1
SRCS+=debug.c
endif

OBJS=$(patsubst %.c,%.o,$(SRCS))
OBJS+=$(patsubst %.s,%.o,$(ASRCS))

all: ext.rom

ext.rom: romdisk.device
	romtool -v build -o $@ -t ext $< test.hdf

romdisk.device: $(OBJS)
	vc $^ -o $@ $(LDFLAGS)

$(SRCS): $(HDRS)

clean:
	rm -f *.o
	rm -f romdisk.device
