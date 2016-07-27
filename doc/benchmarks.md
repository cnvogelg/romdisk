# romdisk Benchmarks

## Enviroment

* SysInfo 4.0 Drive/Speed Test
* A1200 emulated on FS-UAE

### Test1

* Disk: ROMDISK of project with mini s-s
* Buffers 5

| Compressor | Read Speed (Bytes/Sec) |
|---|--:|
| raw  | 3,276,800 |
| nop  | 3,704,208 |
| rnc  | 3,549,866 |
| dflt | 3,407,872 |

* Note: more or less raw copy speed of empty tracks

### Test2

* Disk: Use partial Cloanto wb31.adf
  * C/L Dir
  * Devs without DataTypes, printer.device
* Buffers 5

| Image Size | Image Name |
|-----------:|:-----------|
| 160K | BUILD/disk_dflt.rodi |
| 320K | BUILD/disk_nop.rodi |
| 444K | BUILD/disk_raw.rodi |
| 164K | BUILD/disk_rnc.rodi |
| 196K | BUILD/disk_lz4.rodi |

| Compressor | Read Speed (Bytes/Sec) |
|---|--:|
| raw  | 3,276,800 |
| nop  | 3,549,866 |
| rnc  |  298,936 |
| dflt  | 292,772 |
| lz4  | 916,094 |
