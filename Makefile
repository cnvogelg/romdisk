all: ext.rom

ext.rom: romdisk.device
	romtool build -o $@ -t ext $<

romdisk.device: device.c
	vc $< -o $@ -nostdlib

clean:
	rm -f romdisk.device
