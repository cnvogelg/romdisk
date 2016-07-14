
/* disk header created by mkromdisk and searched in ROM */
struct DiskHeader {
    ULONG   tag; /* RODI tag */
    UWORD   version; /* version of disk header */
    UWORD   format; /* format of disk image */
    UBYTE   name[4]; /* dos name (3 chars + zero!) */
    /* layout of disk */
    ULONG   cylinders;
    ULONG   heads;
    ULONG   sectors;
    ULONG   boot_prio;
    ULONG   dos_type;
};

#define ROMDISK_TAG     0x524f4449 /* RODI */
#define ROMDISK_VERSION 1

extern struct DiskHeader *disk_find(struct DevBase *base);
extern void disk_read(struct IOStdReq *ior, struct DevBase *base);
