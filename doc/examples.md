# romdisk Examples

## 1. A small device-less Kickstart with serial console

### Purpose

We will create a stripped-down OS 3.1 Kickstart for A1200 or A500 that
contains only the essential things that are required to boot from DF0 into
a console.

The fake-trackdisk variant of romdisk is used as a drop-in replacement for
the original trackdisk device. A small disk image is created that mounts
AUX: handler and starts a shell for it.

### Tutorial

#### Split ROM

* First, split an existing ROM using amitool's `romtool into its modules. We use the
  Cloanto Amiga Forever ROM for OS 3.1 on A1200:

```
romtool split -o . amiga-os-310-a1200.rom
```

* A directory called `40.68(A1200)` will be created with all the contained
  modules and an `index.txt` file that contains a list of the modules in the
  order found in the ROM.

* Create a new file `mini.txt` by copying `index.txt` that only contains the
  minimal set of modules:

```
exec_40.10(A1200)
expansion_40.2(A1200)
romboot_40.1
graphics.lib_40.24(AGA)
dos.library_40.3
filesystem_40.1
console.device_40.2
layers.library_40.1
con-handler_40.2
input_40.1
utility.library_40.1(020)
ramlib_40.2
cia.resource_39.1
misc.resource_37.1
potgo.resource_37.4
filesystem.resource_40.1
disk.resource_37.2
timer.device_39.4
keymap.library_40.4
bootmenu_40.5
ram-handler_39.4
shell_40.2
intuition.library_40.85
```

* Note: we also removed `trackdisk.device` since we will replace it with
  romdisk's variant.

#### Setup Disk Image

* Next a ROMDISK disk image will be created: Create a directory called `ROMDISK`
  and build the following disk structure:

```
ROMDISK
 +-- C (Dir)
 |   +-- Mount
 +-- L (Dir)
 |   +-- aux-handler
 +-- Devs (Dir)
 |   +-- serial.device
 |   +-- MountList
 +-- S (Dir)
     +-- Startup-Sequence
```

* Copy the files `C/Mount`, `L/aux-handler`, and `Devs/serial.device` from
  a Workbench 3.1 installation

* `MountList` contains the AUX: mountpoint:

```
AUX:
  Handler     = L:Aux-Handler
  StackSize   = 1000
  Priority    = 5
#
```

* `Startup-Sequence` looks like this:

```
echo "Hello, romboot!"
mount aux:
newshell aux:
```

#### Build Romdisk Image

* Use `mkromdisk` tool of this release to pack the directory into a compressed
  romdisk image. Note: make sure amitool's `xdftool` is installed! We select
  the `deflate` compression method with `-f dflt`.

```
mkromdisk -f dflt -d ROMDISK disk.rodi
```

#### Build ROM Image

* Finally, we combine the modules of the `mini.txt` list with the romdisk
  device and the created romdisk image to create our new Kickstart ROM.
  Note: use the trackdisk.device variant of romdisk!

```
romtool -v build -o kick.rom 40.68(A1200)/mini.txt devs/trackdisk.device_rel_td disk.rodi
```

* Here is the contents of the new ROM:

```
2016-07-25 19:32:44,290  INFO        root        Welcom to romtool
2016-07-25 19:32:44,290  INFO        root        building 512 KiB Kick ROM @00f80000
2016-07-25 19:32:44,291  INFO        root        @00000000: adding module '40.68(A1200)/exec_40.10(A1200)'
2016-07-25 19:32:44,291  INFO        root        @000037b8: adding module '40.68(A1200)/expansion_40.2(A1200)'
2016-07-25 19:32:44,291  INFO        root        @00004290: adding module '40.68(A1200)/romboot_40.1'
2016-07-25 19:32:44,294  INFO        root        @000051a8: adding module '40.68(A1200)/graphics.lib_40.24(AGA)'
2016-07-25 19:32:44,294  INFO        root        @0001e62c: adding module '40.68(A1200)/dos.library_40.3'
2016-07-25 19:32:44,295  INFO        root        @00028238: adding module '40.68(A1200)/filesystem_40.1'
2016-07-25 19:32:44,295  INFO        root        @0002e1d8: adding module '40.68(A1200)/console.device_40.2'
2016-07-25 19:32:44,295  INFO        root        @00031e5c: adding module '40.68(A1200)/layers.library_40.1'
2016-07-25 19:32:44,296  INFO        root        @00035010: adding module '40.68(A1200)/con-handler_40.2'
2016-07-25 19:32:44,296  INFO        root        @000377d4: adding module '40.68(A1200)/input_40.1'
2016-07-25 19:32:44,296  INFO        root        @00038e08: adding module '40.68(A1200)/utility.library_40.1(020)'
2016-07-25 19:32:44,297  INFO        root        @000397c4: adding module '40.68(A1200)/ramlib_40.2'
2016-07-25 19:32:44,297  INFO        root        @00039be8: adding module '40.68(A1200)/cia.resource_39.1'
2016-07-25 19:32:44,297  INFO        root        @00039fd8: adding module '40.68(A1200)/misc.resource_37.1'
2016-07-25 19:32:44,298  INFO        root        @0003a088: adding module '40.68(A1200)/potgo.resource_37.4'
2016-07-25 19:32:44,298  INFO        root        @0003a1c0: adding module '40.68(A1200)/filesystem.resource_40.1'
2016-07-25 19:32:44,298  INFO        root        @0003a360: adding module '40.68(A1200)/disk.resource_37.2'
2016-07-25 19:32:44,298  INFO        root        @0003a6a8: adding module '40.68(A1200)/timer.device_39.4'
2016-07-25 19:32:44,299  INFO        root        @0003b494: adding module '40.68(A1200)/keymap.library_40.4'
2016-07-25 19:32:44,299  INFO        root        @0003c114: adding module '40.68(A1200)/bootmenu_40.5'
2016-07-25 19:32:44,299  INFO        root        @0003d724: adding module '40.68(A1200)/ram-handler_39.4'
2016-07-25 19:32:44,300  INFO        root        @0003fb9c: adding module '40.68(A1200)/shell_40.2'
2016-07-25 19:32:44,307  INFO        root        @0004407c: adding module '40.68(A1200)/intuition.library_40.85'
2016-07-25 19:32:44,312  INFO        root        @0005da1c: adding module 'devs/trackdisk.device_rel_td'
2016-07-25 19:32:44,312  INFO        root        @0005eb50: adding raw data 'disk.rodi'
2016-07-25 19:32:44,312  INFO        root        @00061e98: padding 123216 bytes with ff
2016-07-25 19:32:44,407  INFO        root        saving ROM to 'kick.rom'
```

* Now use your favorite Amiga emulator to test the ROM or soft kick it on a
  real machine! You should see a CLI on both the boot console and on serial.
