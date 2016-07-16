
all: ext_raw.rom ext_rnc.rom

debug:
	$(MAKE) all DEBUG=1

ext_raw.rom: BUILD/romdisk.device disk_raw.rodi
	romtool -v build -o $@ -t ext $^

ext_rnc.rom: BUILD/romdisk.device disk_rnc.rodi
	romtool -v build -o $@ -t ext $^

BUILD/romdisk.device:
	$(MAKE) -C src

disk_raw.rodi: ROMDISK
	./mkromdisk $@ -d ROMDISK

disk_rnc.rodi: ROMDISK
	./mkromdisk $@ -d ROMDISK -f rnc

clean_all: clean
	rm -rf BUILD

clean:
	rm -f disk_raw.rodi disk_rnc.rodi
	rm -f ext_raw.rom ext_rnc.rom
