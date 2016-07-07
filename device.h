#define VERSION     1
#define REVISION    1
#define VER_STR     "1.1"
#define DATE        "04.07.2016"
#define NAME        "romdisk.device"

struct DevBase
{
  struct Library  libBase;
  struct Library *sysBase;
  struct Library *expansionBase;
  ULONG           segList;
};

#define SysBase base->sysBase
#define ExpansionBase base->expansionBase
