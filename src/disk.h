
/* disk header created by mkromdisk and searched in ROM */
struct DiskHeader {
    ULONG   tag;            /* RODI tag */
    UWORD   version;        /* version of disk header */
    UWORD   format;         /* format of disk image */
    UBYTE   name[4];        /* dos name (3 chars + zero!) */
    /* layout of disk */
    ULONG   cylinders;
    ULONG   heads;
    ULONG   sectors;
    ULONG   boot_prio;
    ULONG   dos_type;
    /* params */
    ULONG   num_buffers;    /* number of RAM buffers for caching */
    ULONG   disk_size;      /* total bytes in disk */
};

/* placed in ROM after DiskHeader if format is packed */
struct PackHeader {
    ULONG   tag;            /* PACK tag */
    ULONG   packer;         /* tag for packer */
    ULONG   num_packs;      /* number of packs (per track/per cylinder) */
    ULONG   pack_size;      /* size of each pack in bytes */
    ULONG   offsets[1];     /* num_packs offsets following */
};

#define ROMDISK_TAG     0x524f4449 /* RODI */
#define ROMDISK_VERSION 1

#define ROMDISK_FORMAT_RAW      0
#define ROMDISK_FORMAT_PACK     1

#define ROMDISK_PACK_TAG 0x5041434b /* PACK */
#define ROMDISK_PACK_RNC 0x524e4300 /* RNC */
#define ROMDISK_PACK_NOP 0x4e4f5000 /* NOP */
#define ROMDISK_PACK_DFLT 0x44464c54 /* DFLT */
#define ROMDISK_PACK_LZ4 0x4c5a3400

extern BOOL disk_setup(struct DevBase *base);
extern BOOL disk_open(struct DevBase *base);
extern void disk_close(struct DevBase *base);
