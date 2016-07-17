FORMAT?=raw
ROM_NAME=ext_$(FORMAT).rom
DISK_NAME=disk_$(FORMAT).rodi

all: raw rnc

rom: $(ROM_NAME)

raw:
	$(MAKE) FORMAT=raw rom

rnc:
	$(MAKE) FORMAT=rnc rom

debug:
	$(MAKE) all DEBUG=1

$(ROM_NAME): BUILD/romdisk.device $(DISK_NAME)
	romtool -v build -o $@ -t ext $^

BUILD/romdisk.device:
	$(MAKE) -C src

$(DISK_NAME): ROMDISK
	./mkromdisk $@ -d ROMDISK -D ofs -p 15 -f $(FORMAT)

clean_all: clean
	rm -rf BUILD

clean:
	rm -f disk_raw.rodi disk_rnc.rodi
	rm -f ext_raw.rom ext_rnc.rom
