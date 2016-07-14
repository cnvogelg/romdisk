#include <proto/exec.h>
#include <devices/trackdisk.h>

#include "debug.h"
#include "mydev.h"
#include "disk.h"

const ULONG disk_size = 0x80000;
extern ULONG theEnd;

struct DiskHeader *disk_find(struct DevBase *base)
{
  /* HACK: point after device in ROM */
  ULONG *ptr = &theEnd + 1;
  D(("disk_find: start=%08lx\n", ptr));

  /* search range: 1 KiB */
  int i;
  for(i=0;i<256;i++) {
    if(*ptr == ROMDISK_TAG) {
      struct DiskHeader *hdr = (struct DiskHeader *)ptr;
      if(hdr->version == ROMDISK_VERSION) {
        D(("disk_find: found=%08lx\n", ptr));
        return hdr;
      }
    }
    ptr++;
  }

  D(("disk_find: NOT FOUND!\n"));
  return NULL;
}

void disk_read(struct IOStdReq *ior, struct DevBase *base)
{
  ULONG offset = ior->io_Offset;
  ULONG length = ior->io_Length;
  APTR  buffer = ior->io_Data;

  if((offset + length) > disk_size) {
    ior->io_Actual = 0;
    ior->io_Error = TDERR_NotSpecified;
    D(("read out of range off=%08lx len=%08lx\n", offset, length));
  } else {
    ior->io_Actual = length;
    UBYTE *data = base->diskData + offset;
    D(("read data @%08lx\n", data));
    CopyMemQuick(data, buffer, length);
  }
}
