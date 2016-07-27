# Project romdisk - Boot your Amiga from ROM

Written by Christian Vogelgsang under the GNU Public License V2

This projects provides a romdisk.device you can place in an Amiga Kickstart
or external ROM. It will autoboot a disk image that is stored next to the
device directly in the ROM. You can either store a raw OFS /FFS disk image
(ca. 500 KiB size) or a compressed image that holds up to a whole Amiga
disk (ca. 880 KiB size). While the raw storage is ultra fast, the compressed
images allow you to deploy whole disks into ROM.

Additionally, a tool for modern systems is available that allows you to
create those romdisk image easily from existing .adf disk images or from
a directory of files.

## Why?

Create boot ROMs for

* Diagnosis
* Network Boot
* Quick Startup
* Device-less Amiga Setups
* stripped down Amiga OS ROMs that do not use any external devices
  during boot (simplifies emulation)

and

* use them in your favorite Amiga emulator
* soft-kick'em in your accelerator card
* flash or burn them to ROMs

## Features

* Store raw or compressed disk images
* Compressions supported:
  * Deflate Compression from ZLib
  * RNC Compressor (Rob Nothern Computing)
  * LZ4 Fast Compression
* Autoboot on OS 2.x and 3.x
* Configurable image geometry, boot priority
* Python tool `mkromdisk` to create image file from disk images or directories
* Amiga device with source that can be cross-compiled on modern systems
* A drop-in replacement variant for `trackdisk.device` to have a very
  compatible DF0 device

## Prerequisites

* Amiga OS 2.x or 3.x and real/emulated classic Amiga m68k system
* Python 2.7 for host tool
* [amitools][1] installed for `xdftool`, `romtool`, and `vamos`
* `ppami.exe` Amiga tool from [RNC ProPack][2] to use RNC Compression
* `lz4` tool from [lz4.org][4] to use LZ4 Compression
* Cross compilers from [amigaos-cross-toolchain][3] to build device from sources

[1]: https://github.com/cnvogelg/amitools
[2]: http://aminet.net/package/util/pack/RNC_ProPack
[3]: https://github.com/cahirwpz/amigaos-cross-toolchain
[4]: https://lz4.org/

## Tutorial

Here is a quick wrap-up on how to create and run a ROM with a romdisk:

* First create a Kickstart ROM that supports an external ROM with additional
  512 KiB storage. We use amitool's `romtool` to convert a A1200 Cloanto OS 3.1
  to support 1 MiB ROMs:

```
romtool patch -o kick.rom miga-os-310-a1200.rom 1mb_rom
```

* Now run romdisk's `mkromdisk` tool to create a romdisk image from a disk
  image. We use the Workbench 3.1 disk from Cloanto's Amiga Forever distribution
  here (its called `amiga-os-310-workbench.adf`):

```
./mkromdisk -i amiga-os-310-workbench.adf -f dflt wb31.rodi
```

* Finally, create an external ROM image with `romtool` that contains the
`romdisk.device` (release version) and your romdisk image `wb31.rodi`:

```
romtool -v build -o ext.rom -t ext romdisk.device_rel wb31.rodi
```

* Voila! Your ROM set is ready to be run. You can either use the two files
directly (e.g. in your emulator) or combine them to a single 1 MiB ROM file
if your soft-kicker needs that:

```
romtool combine -o 1mb.rom kick.rom ext.rom
```

* Run the ROM set in the [FS-UAE][1] emulator with a configuration like this
one (e.g. in a `test.fs-uae` file):

```
[config]
amiga_model = A1200
kickstart_file = kick.rom
kickstart_ext_file = ext.rom
```

If you run the emulator it will boot the Workbench directly from ROM...

[1]: https://fs-uae.net

## More Documents

* [romdisk Examples](doc/examples.md)
* [romdisk Benchmarks](doc/benchmarks.md)
* [mkromdisk Manual](doc/mkromdisk.md)

## Thanks

* Krystian Bacławski for the great [amigaos-cross-toolchain][1]
* Keir Fraser for releasing the [deflate m68k source][2]
* Frank Wille for [vasm][3] and porting the deflate source to vasm
* Rob Northern Computing for releasing its [Pro Pack][4] including source

[1]: https://github.com/cahirwpz/amigaos-cross-toolchain
[2]: https://github.com/keirf/Amiga-Stuff/blob/master/inflate.asm
[3]: http://sun.hasenbraten.de/vasm/
[4]: http://aminet.net/package/util/pack/RNC_ProPack
