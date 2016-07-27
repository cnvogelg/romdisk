# The mkromdisk Manual

## Introduction

The mkromdisk is the host-based tool of the romdisk project that allows you
to create disk images that are suitable to be placed into a ROM and used
as a romdisk with the romdisk.device of this project.

You can either create a romdisk image directly **from a directory tree** (in this
case amitool's xdftool is used to create a Amiga OFS/FFS image first) or you
supply a **pre-mastered disk image** in a .adf or .hdf file.

Both floppy disk images (usually stored in .adf files) and hard disk files of
arbitrary disk geometry (stored in .hdf files) are supported. Since ROMs have
a size of 512 KiB floppy disk images can only be stored in a compressed
format. Hard disk images with a very small geometry (e.g. 40 cylinders, 2
heads, 11 sectors = half a disk = 440 KiB) also fit in a ROM uncompressed.

The mkromdisk tool lets you decide if and what type of compression you want
to apply. Furthermore, you can select parameters of your drive, like boot
priority or number of file system buffers used.

Depending on the type of compression you selected, you will need to install
an external compression tool on your command line first.

The tool is written in **Python 2.7** (not 3.x!), so make sure you have it
installed on your machine.

## Typical Usages

### From Directory

* Create a romdisk directly from a directory tree (Assuming `ROMDISK` is a
  directory that contains all the files and directories for your romdisk):

```
mkromdisk -d ROMDISK disk.rodi
```

  * Note: The romdisk disks use a `.rodi` suffix and the tool always requires
    the name of a output disk image as argument.

### From Disk Image

* Create a romdisk from a pre-mastered disk image:

```
mkromdisk -i wb31.adf -f dflt disk.rodi
```

  * Note: since a floppy disk image does not fit uncompressed, we enable
    compression with the -f option

## Options

The tool allows you to specify lots of additional options to alter its
behavior.

* `-h` shows you detailed help on all switches
* `-d <directory>` create a romdisk image from a given directory.
  Make sure to install `xdftool` command from amitools, first.
* `-i <file.xdf>` create a romdisk image from a given disk image.
* `-t <prefix>` during image creation some temporary files are created. This
  option sets the name prefix used for all files. Default is `temp`.
* `-g <cyls>,<heads>,<secs>` specify a disk image geometry. Specify cylinders,
  heads, and sectors as numbers, e.g. `-g 80,2,11` specifies a floppy disk
  geometry. Use `-g adf` as a short cut for floppies.
* `-D <dostype>` when creating an image from a directory (`-i`), you can specify
  the created file system with this option: Use `ofs` for old and `ffs` for
  fast file system images. Default is `ffs`.
* `-p <prio>` set the boot priority of the romdisk. Default is `5`.
* `-b <num_buffers>` set the number of buffers that the filesys uses. Default is `5`.
* `-f <format>` sets the compression format (Default is `raw`):
  * `raw`: no compression applied at all. disk is stored unmodified. even empty
    tracks are kept.
  * `nop`: no compression, but omit empty (zero) tracks.
  * `dflt`: compress with zlib's deflate compressor (level 9). no external
    tool is required.
  * `rnc`: compress with Rob Northern Computing's ProPack. You need to install
    `vamos` from amitools first and supply the packer binary `ppami.exe` in
    current dir.
  * `lz4`: compress with lz4 algorithm. You need to install the `lz4` tool
    first.
* `-v` be more verbose
* `-e <entity>` select the compression granularity: either pack whole `tracks`
  or `cyls` (= track * heads). Cylinder packing needs to decompress more for
  a single block but allows you to usually get a better compression ratio.
  Default is `tracks`.
