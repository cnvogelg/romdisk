#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <exec/errors.h>
#include <dos/dos.h>
#include <devices/trackdisk.h>

#include <proto/exec.h>
#include <proto/expansion.h>

#include <SDI/SDI_compiler.h>

#include "debug.h"
#include "device.h"
#include "boot.h"
#include "disk.h"

#define VERS        NAME " " VER_STR
#define VSTRING     NAME " " VER_STR " (" DATE ")\r\n"
#define VERSTAG     "\0$VER: " NAME " " VER_STR " (" DATE ")"

int Main(void)
{
  return RETURN_FAIL;
}

static const char UserLibName[] = NAME;
static const char UserLibVer[]  = VSTRING;
static const char UserLibID[]   = VERSTAG;

#define LIBFUNC SAVEDS ASM

LIBFUNC static struct DevBase * DevInit    (REG(a0, BPTR Segment),
                                            REG(d0, struct DevBase *lh),
                                            REG(a6, struct ExecBase *sb));
LIBFUNC static BPTR             DevExpunge (REG(a6, struct DevBase *base));
LIBFUNC static struct DevBase * DevOpen    (REG(a1, struct IOStdReq *ior),
                                            REG(d0, ULONG unit),
                                            REG(d1, ULONG flags),
                                            REG(a6, struct DevBase *base));
LIBFUNC static BPTR             DevClose   (REG(a1, struct IOStdReq *ior),
                                            REG(a6, struct DevBase *base));
LIBFUNC static LONG             DevNull    (void);
LIBFUNC static void             DevBeginIO(REG(a1, struct IOStdReq *ior),
                                           REG(a6, struct DevBase *base));
LIBFUNC static LONG             DevAbortIO(REG(a1, struct IOStdReq *ior),
                                           REG(a6, struct DevBase *base));

static const APTR LibVectors[] =
{
  (APTR)DevOpen,
  (APTR)DevClose,
  (APTR)DevExpunge,
  (APTR)DevNull,
  (APTR)DevBeginIO,
  (APTR)DevAbortIO,
  (APTR)-1
};

static const ULONG LibInitTab[] =
{
  sizeof(struct DevBase),
  (ULONG)LibVectors,
  (ULONG)NULL,
  (ULONG)DevInit
};

/* ---- RomTag ---- */
static const USED_VAR struct Resident ROMTag =
{
  RTC_MATCHWORD,
  (struct Resident *)&ROMTag,
  (struct Resident *)&ROMTag + 1,
  RTF_AUTOINIT | RTF_COLDSTART,
  VERSION,
  NT_DEVICE,
  0, /* prio */
  (APTR)UserLibName,
  (APTR)UserLibVer,
  (APTR)LibInitTab
};

#define DeleteLibrary(LIB) \
  FreeMem((STRPTR)(LIB)-(LIB)->lib_NegSize, (ULONG)((LIB)->lib_NegSize+(LIB)->lib_PosSize))

/* ----- Functions ----- */

LIBFUNC static struct DevBase * DevInit(REG(a0, BPTR Segment),
                                        REG(d0, struct DevBase *base),
                                        REG(a6, struct ExecBase *sb))
{
  base->libBase.lib_Node.ln_Type = NT_LIBRARY;
  base->libBase.lib_Node.ln_Pri  = 0;
  base->libBase.lib_Node.ln_Name = (char *)UserLibName;
  base->libBase.lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  base->libBase.lib_Version      = VERSION;
  base->libBase.lib_Revision     = REVISION;
  base->libBase.lib_IdString     = (char *)UserLibVer;

  base->segList = Segment;
  base->sysBase = (APTR)sb;

  disk_init(base);
  boot_init(base);

  return base;
}

LIBFUNC static BPTR DevExpunge(REG(a6, struct DevBase *base))
{
  BPTR rc;

  if(base->libBase.lib_OpenCnt > 0)
  {
    base->libBase.lib_Flags |= LIBF_DELEXP;
    return 0;
  }

  rc = base->segList;

  Remove((struct Node *)base);
  DeleteLibrary(&base->libBase);

  return rc;
}

LIBFUNC static struct DevBase * DevOpen(REG(a1, struct IOStdReq *ior),
                                        REG(d0, ULONG unit),
                                        REG(d1, ULONG flags),
                                        REG(a6, struct DevBase *base))
{
  D(("DevOpen(%lx,%ld,%ld)\n", ior, unit, flags));
  base->libBase.lib_Flags &= ~LIBF_DELEXP;
  base->libBase.lib_OpenCnt++;

  return base;
}

LIBFUNC static BPTR DevClose(REG(a1, struct IOStdReq *ior),
                             REG(a6, struct DevBase *base))
{
  D(("DevClose(%lx)\n", ior));
  if(base->libBase.lib_OpenCnt > 0 &&
     --base->libBase.lib_OpenCnt == 0)
  {
    if(base->libBase.lib_Flags & LIBF_DELEXP)
    {
      return DevExpunge(base);
    }
  }

  return 0;
}

LIBFUNC static LONG DevNull(void)
{
  return 0;
}

LIBFUNC static void DevBeginIO(REG(a1, struct IOStdReq *ior),
                               REG(a6, struct DevBase *base))
{
  UWORD cmd = ior->io_Command;
  D(("DevBeginIO(%lx) cmd=%lx\n", ior, cmd));

  /* assume no error */
  ior->io_Error = 0;

  switch(cmd) {
    case CMD_READ:
      D(("READ: off=%08lx len=%08lx buf=%08lx\n",
         ior->io_Offset, ior->io_Length, ior->io_Data));
      disk_read(ior, base);
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
  ior->io_Message.mn_Node.ln_Type = NT_MESSAGE;
  if (!(ior->io_Flags & IOF_QUICK)) {
    ReplyMsg(&ior->io_Message);
  }
}

LIBFUNC static LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
                               REG(a6, struct DevBase *base))
{
  D(("DevAbortIO(%lx)\n", ior));
  return 1;
}
