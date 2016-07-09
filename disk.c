#include <proto/exec.h>
#include <devices/trackdisk.h>

#include "debug.h"
#include "device.h"

ULONG disk_size = 0x80000;

void disk_init(struct DevBase *base)
{
  /* HACK: point after device in ROM */
  base->disk_addr = (UBYTE *)&disk_size + 4;
  D(("disk_addr=%08lx\n", base->disk_addr));
}

void disk_read(struct IOStdReq *ior, struct DevBase *base)
{
  ULONG offset = ior->io_Offset;
  ULONG length = ior->io_Length;
  APTR  buffer = ior->io_Data;

  if((offset + length) > disk_size) {
    ior->io_Actual = 0;
    ior->io_Error = TDERR_NotSpecified;
  } else {
    ior->io_Actual = length;
    UBYTE *data = base->disk_addr + offset;
    D(("read data @%08lx\n", data));
    CopyMemQuick(data, buffer, length);
  }
}
