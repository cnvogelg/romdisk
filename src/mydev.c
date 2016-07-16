#include <exec/types.h>
#include <devices/trackdisk.h>
#include <exec/errors.h>

#include <proto/exec.h>
#include <proto/expansion.h>

#include <SDI/SDI_compiler.h>

#include "debug.h"
#include "mydev.h"
#include "boot.h"
#include "disk.h"

struct DevBase *mydev_init(struct DevBase *base)
{
  /* try to find disk in ROM */
  if(!disk_setup(base)) {
    return NULL;
  }

  boot_init(base);

  return base;
}

void mydev_expunge(struct DevBase *base)
{

}

struct DevBase * mydev_open(struct IOStdReq *ior, ULONG unit, ULONG flags, struct DevBase *base)
{
  /* only unit 0 allowed */
  if(unit != 0) {
    D(("only unit 0 allowed!\n"));
    return NULL;
  }

  if(base->libBase.lib_OpenCnt == 1) {
    D(("disk_open\n"));
    if(!disk_open(base)) {
      D(("disk_open failed!\n"));
      return NULL;
    }
  }

  return base;
}

void mydev_close(struct IOStdReq *ior, struct DevBase *base)
{
  if(base->libBase.lib_OpenCnt == 1) {
    D(("disk_close\n"));
    disk_close(base);
  }
}

void mydev_begin_io(struct IOStdReq *ior, struct DevBase *base)
{
  /* assume no error */
  ior->io_Error = 0;

  UWORD cmd = ior->io_Command;
  switch(cmd) {
    case CMD_READ:
      D(("READ: off=%08lx len=%08lx buf=%08lx\n",
         ior->io_Offset, ior->io_Length, ior->io_Data));
      base->readFunc(ior, base);
      break;
    case TD_CHANGENUM:
    case TD_CHANGESTATE:
      ior->io_Actual = 0;
      break;
    case TD_PROTSTATUS:
      ior->io_Actual = 1; /* is write protected */
      break;
      /* ignore the following commands */
    case CMD_UPDATE:
    case CMD_CLEAR:
    case TD_MOTOR:
    case TD_SEEK:
    case TD_REMOVE:
      D(("NOP\n"));
      break;
      /* report invalid write */
    case CMD_WRITE:
    case TD_FORMAT:
      D(("Write!?\n"));
      ior->io_Error = TDERR_WriteProt;
      break;
      /* report invalid command */
    default:
      D(("??\n"));
      ior->io_Error = IOERR_NOCMD;
      break;
  }

  /* reply message */
  D(("  Reply\n"));
  ior->io_Message.mn_Node.ln_Type = NT_MESSAGE;
  if (!(ior->io_Flags & IOF_QUICK)) {
    ReplyMsg(&ior->io_Message);
  }
  D(("  Done\n"));
}

LONG mydev_abort_io(struct IOStdReq *ior, struct DevBase *base)
{
  return 1;
}
