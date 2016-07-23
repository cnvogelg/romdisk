#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/expansion.h>

#include "compiler.h"
#include "debug.h"
#include "mydev.h"

#define VERS        MYDEV_NAME " " MYDEV_VERSION_STR
#define VSTRING     MYDEV_NAME " " MYDEV_VERSION_STR " (" MYDEV_DATE ")\r\n"
#define VERSTAG     "\0$VER: " MYDEV_NAME " " MYDEV_VERSION_STR " (" MYDEV_DATE ")"

int Main(void)
{
  return RETURN_FAIL;
}

static const char UserLibName[] = MYDEV_NAME;
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
static const struct Resident ROMTag =
{
  RTC_MATCHWORD,
  (struct Resident *)&ROMTag,
  (struct Resident *)&ROMTag + 1,
  RTF_AUTOINIT | RTF_COLDSTART,
  MYDEV_VERSION,
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
  base->libBase.lib_Version      = MYDEV_VERSION;
  base->libBase.lib_Revision     = MYDEV_REVISION;
  base->libBase.lib_IdString     = (char *)UserLibVer;

  base->segList = Segment;
  base->sysBase = (APTR)sb;

  D(("+DevInit(%08lx, %08lx, %08lx)\n", Segment, base, sb));
  struct DevBase *result = mydev_init(base);
  D(("-DevInit: result=%08lx\n", result));
  return result;
}

LIBFUNC static BPTR DevExpunge(REG(a6, struct DevBase *base))
{
  BPTR rc;

  if(base->libBase.lib_OpenCnt > 0)
  {
    base->libBase.lib_Flags |= LIBF_DELEXP;
    return 0;
  }

  mydev_expunge(base);

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
  D(("+DevOpen(%lx,%ld,%ld)\n", ior, unit, flags));
  if(base == NULL) {
    return NULL;
  }

  base->libBase.lib_OpenCnt++;

  struct DevBase *result = mydev_open(ior, unit, flags, base);

  if(result == NULL) {
    base->libBase.lib_OpenCnt--;
  } else {
    base->libBase.lib_Flags &= ~LIBF_DELEXP;
  }

  D(("-DevOpen: result=%08lx\n", result));
  return result;
}

LIBFUNC static BPTR DevClose(REG(a1, struct IOStdReq *ior),
                             REG(a6, struct DevBase *base))
{
  BPTR result = 0;
  D(("+DevClose(%lx)\n", ior));

  mydev_close(ior, base);

  base->libBase.lib_OpenCnt--;
  if(base->libBase.lib_OpenCnt == 0)
  {
    if(base->libBase.lib_Flags & LIBF_DELEXP)
    {
      result = DevExpunge(base);
    }
  }

  D(("-DevClose: result=%08lx\n", result));
  return 0;
}

LIBFUNC static LONG DevNull(void)
{
  return 0;
}

LIBFUNC static void DevBeginIO(REG(a1, struct IOStdReq *ior),
                               REG(a6, struct DevBase *base))
{
  D(("+DevBeginIO(%lx) cmd=%lx\n", ior, ior->io_Command));
  mydev_begin_io(ior, base);
  D(("-DevBeginIO(%lx)\n", ior));
}

LIBFUNC static LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
                               REG(a6, struct DevBase *base))
{
  D(("DevAbortIO(%lx)\n", ior));
  return mydev_abort_io(ior, base);
}
