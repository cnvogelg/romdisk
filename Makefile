
all: ext.rom

ext.rom: BUILD/romdisk.device sample.rodi
	romtool -v build -o $@ -t ext $^

BUILD/romdisk.device:
	$(MAKE) -C src

sample.rodi: ROMDISK
	./mkromdisk $@ -d ROMDISK

clean_all: clean
	rm -rf BUILD

clean:
	rm -f sample.rodi
	rm -f ext.rom
