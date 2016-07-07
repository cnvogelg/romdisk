#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <dos/dos.h>

#include <proto/exec.h>

#include <SDI/SDI_compiler.h>

#define VERSION     1
#define REVISION    1
#define VER_STR     "1.1"
#define DATE        "04.07.2016"
#define NAME        "romdisk.device"

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

struct LibraryHeader
{
  struct Library  libBase;
  struct Library *sysBase;
  ULONG           segList;
};

#define SysBase base->sysBase

#define LIBFUNC SAVEDS ASM

LIBFUNC static struct LibraryHeader * DevInit    (REG(a0, BPTR Segment),
                                                  REG(d0, struct LibraryHeader *lh),
                                                  REG(a6, struct ExecBase *sb));
LIBFUNC static BPTR                   DevExpunge (REG(a6, struct LibraryHeader *base));
LIBFUNC static struct LibraryHeader * DevOpen    (REG(a1, struct IOStdReq *ior),
                                                  REG(d0, ULONG unit),
                                                  REG(d1, ULONG flags),
                                                  REG(a6, struct LibraryHeader *base));
LIBFUNC static BPTR                   DevClose   (REG(a1, struct IOStdReq *ior),
                                                  REG(a6, struct LibraryHeader *base));
LIBFUNC static LONG                   DevNull    (void);
LIBFUNC static void                   DevBeginIO(REG(a1, struct IOStdReq *ior),
                                                 REG(a6, struct LibraryHeader *base));
LIBFUNC static LONG                   DevAbortIO(REG(a1, struct IOStdReq *ior),
                                                 REG(a6, struct LibraryHeader *base));

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
  sizeof(struct LibraryHeader),
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
  0,
  (APTR)UserLibName,
  (APTR)UserLibVer,
  (APTR)LibInitTab
};

#define DeleteLibrary(LIB) \
  FreeMem((STRPTR)(LIB)-(LIB)->lib_NegSize, (ULONG)((LIB)->lib_NegSize+(LIB)->lib_PosSize))

/* ----- Functions ----- */

LIBFUNC static struct LibraryHeader * DevInit(REG(a0, BPTR Segment),
                                              REG(d0, struct LibraryHeader *base),
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

  return base;
}

LIBFUNC static BPTR DevExpunge(REG(a6, struct LibraryHeader *base))
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

LIBFUNC static struct LibraryHeader * DevOpen(REG(a1, struct IOStdReq *ior),
                                              REG(d0, ULONG unit),
                                              REG(d1, ULONG flags),
                                              REG(a6, struct LibraryHeader *base))
{
  base->libBase.lib_Flags &= ~LIBF_DELEXP;
  base->libBase.lib_OpenCnt++;

  return base;
}

LIBFUNC static BPTR DevClose(REG(a1, struct IOStdReq *ior),
                             REG(a6, struct LibraryHeader *base))
{
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
                               REG(a6, struct LibraryHeader *base))
{

}

LIBFUNC static LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
                               REG(a6, struct LibraryHeader *base))
{
  return 0;
}
