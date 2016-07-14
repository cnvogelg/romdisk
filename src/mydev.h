#define VERSION     1
#define REVISION    1
#define VER_STR     "1.1"
#define DATE        "04.07.2016"
#define NAME        "romdisk.device"

struct DiskHeader;

struct DevBase
{
  struct Library  libBase;
  struct Library *sysBase;
  ULONG           segList;
  struct DiskHeader *diskHeader;
  UBYTE          *diskData;
};

#define SysBase base->sysBase

extern struct DevBase *mydev_init(struct DevBase *base);
extern void mydev_expunge(struct DevBase *base);
extern struct DevBase * mydev_open(struct IOStdReq *ior, ULONG unit, ULONG flags, struct DevBase *base);
extern void mydev_close(struct IOStdReq *ior, struct DevBase *base);
extern void mydev_begin_io(struct IOStdReq *ior, struct DevBase *base);
extern LONG mydev_abort_io(struct IOStdReq *ior, struct DevBase *base);
