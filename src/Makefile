# Makefile to cross-compile romdisk.device

# toolchain dire
TC_DIR=/opt/m68k-amigaos

# enable debugging?
DEBUG=1

CC=vc
CFLAGS=-c99 -g -sc
LDFLAGS=-g -nostdlib
AS=vasmm68k_mot
ASFLAGS=-Fhunk -I$(TC_DIR)/os-include

SRCS=device.c boot.c disk.c mydev.c
ASRCS=diag.s
HDRS=boot.h disk.h debug.h mydev.h

ifeq "$(DEBUG)" "1"
CFLAGS+=-DDEBUG=1
SRCS+=debug.c
endif

BUILD_DIR=../BUILD

OBJS=$(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
OBJS+=$(patsubst %.s,$(BUILD_DIR)/%.o,$(ASRCS))

DEVICE=$(BUILD_DIR)/romdisk.device

all: $(BUILD_DIR) $(DEVICE)

$(DEVICE): $(OBJS)
	vc $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SRCS): $(HDRS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
