FORMAT?=raw
FLAVOR?=_rel
BUILD_DIR=BUILD
ROM_NAME=$(BUILD_DIR)/ext_$(FORMAT)$(FLAVOR).rom
DISK_NAME=$(BUILD_DIR)/disk_$(FORMAT).rodi

DEV_NAME=src/BUILD/romdisk.device$(FLAVOR)

FLAVORS=_rel _dbg
FORMATS=raw rnc

all: $(BUILD_DIR) $(ROM_NAME)

formats:
	@for f in $(FORMATS) ; do \
		$(MAKE) FORMAT=$$f ; \
	done
	@echo "--- formats $(FORMATS) ---"

flavors:
	@for f in $(FLAVORS) ; do \
		$(MAKE) FLAVOR=$$f formats ; \
	done
	@echo "--- flavors $(FLAVORS) ---"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEV_NAME):
	$(MAKE) -C src FLAVOR=$(FLAVOR)

$(DISK_NAME): ROMDISK
	./mkromdisk $@ -d ROMDISK -p 15 -f $(FORMAT)

$(ROM_NAME): $(DEV_NAME) $(DISK_NAME)
	romtool -v build -o $@ -t ext $^

clean_all: clean
	$(MAKE) -C src clean

clean:
	rm -rf BUILD
