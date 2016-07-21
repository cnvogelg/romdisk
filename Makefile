FORMAT?=raw
FLAVOR?=_rel
BUILD_DIR=BUILD
ROM_NAME=$(BUILD_DIR)/ext_$(FORMAT)$(FLAVOR).rom
DISK_NAME=$(BUILD_DIR)/disk_$(FORMAT).rodi

DEV_NAME=src/BUILD/romdisk.device$(FLAVOR)

FLAVORS=_rel _dbg
FORMATS=raw rnc

include version.mk

DIST_NAME=romdisk_$(PROJECT_MAJOR).$(PROJECT_MINOR)

all: $(ROM_NAME)

formats:
	@for f in $(FORMATS) ; do \
		$(MAKE) FORMAT=$$f || exit 1 ; \
	done
	@echo "--- formats $(FORMATS) ---"

flavors:
	@for f in $(FLAVORS) ; do \
		$(MAKE) FLAVOR=$$f formats || exit 1 ; \
	done
	@echo "--- flavors $(FLAVORS) ---"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEV_NAME):
	$(MAKE) -C src FLAVOR=$(FLAVOR)

$(DISK_NAME): $(BUILD_DIR) ROMDISK
	./mkromdisk $@ -d ROMDISK -p 15 -f $(FORMAT)

$(ROM_NAME): $(DEV_NAME) $(DISK_NAME)
	romtool -v build -o $@ -t ext $^

clean_all: clean clean_dist
	$(MAKE) -C src clean
	rm -rf DIST

clean:
	rm -rf BUILD

dist_dirs:
	@mkdir -p $(DIST_NAME)/devs $(DIST_NAME)/roms

dist_flavors:
	@for f in $(FLAVORS) ; do \
		$(MAKE) FLAVOR=$$f dist_formats || exit 1 ; \
	done

dist_formats:
	@for f in $(FORMATS) ; do \
		$(MAKE) FORMAT=$$f dist_local || exit 1 ; \
	done

dist_local: $(ROM_NAME)
	cp $(ROM_NAME) $(DIST_NAME)/roms

dist: dist_dirs dist_flavors
	@$(MAKE) -C src dist DIST_DIR=../$(DIST_NAME)
	@cp README.md $(DIST_NAME)/
	@echo "--- dist: $(DIST_NAME) ---"
	@ls -laR $(DIST_NAME)

dist_zip: dist
	@zip -r -9 $(DIST_NAME).zip $(DIST_NAME)

clean_dist:
	rm -rf $(DIST_NAME)
	rm -f $(DIST_NAME).zip
