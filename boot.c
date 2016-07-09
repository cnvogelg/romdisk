#define __NOLIBBASE__

#include <proto/exec.h>
#include <proto/expansion.h>

#include <exec/libraries.h>
#include <libraries/expansion.h>

#include "debug.h"
#include "device.h"

#include <SDI/SDI_compiler.h>

static const char execName[] = "romdisk.device";
static const char dosName[] = "rom";

extern struct DiagArea myDiagArea;

const ULONG parmPkt[] = {
  (ULONG)dosName,
  (ULONG)execName,
  0, /* unit */
  0, /* open flags */

  /* env block */
  16,                 /* size of table */
  512>>2,             /* 0 # longwords in a block */
  0,                  /* 1 sector origin -- unused */
  2,                  /* 2 number of surfaces */
  1,                  /* 3 secs per logical block -- leave as 1 */
  11,                 /* 4 blocks per track */
  2,                  /* 5 reserved blocks -- 2 boot blocks */
  0,                  /* 6 ?? -- unused */
  0,                  /* 7 interleave */
  0,                  /* 8 lower cylinder */
  39,                 /* 9 upper cylinder */
  5,                  /* a number of buffers */
  MEMF_PUBLIC,        /* b type of memory for buffers */
  (~0 >> 1),          /* c largest transfer size (largest signed #) */
  ~1,                 /* d addmask */
  0,                  /* e boot priority */
  0x444f5300          /* f dostype: 'DOS\0' */
};

BOOL boot_init(struct DevBase *base)
{
  BOOL ok = FALSE;
  struct Library *ExpansionBase;

  ExpansionBase = (struct Library *)OpenLibrary("expansion.library", 36);
  if(ExpansionBase != NULL) {
    struct ConfigDev *cd = AllocConfigDev();
    D(("got expansion. config dev=%08lx\n", cd));
    if(cd != NULL) {

      /* get diag address */
      ULONG diag_addr = (ULONG)&myDiagArea;
      ULONG diag_base = diag_addr & ~0xffff;
      ULONG diag_off  = diag_addr & 0xffff;
      D(("diag_addr: base=%08lx offset=%04lx\n", diag_base, diag_off));

      /* fill faked config dev */
      cd->cd_Flags = 0;
      cd->cd_BoardAddr = (APTR)diag_base;
      cd->cd_BoardSize = 0x010000;
      cd->cd_Driver = (APTR)base;
      struct ExpansionRom *rom = &cd->cd_Rom;
      rom->er_Type = ERT_ZORROII | ERTF_DIAGVALID | 1; /* size=64 KiB */
      rom->er_Flags = ERFF_NOSHUTUP;
      rom->er_Product = 42;
      rom->er_Manufacturer = 2011; /* hack id */
      rom->er_SerialNumber = 1;
      rom->er_InitDiagVec = (UWORD)diag_off;

      /* fake copy of diag area. the pointer is stored in er_Reserved0c..0f */
      ULONG *ptr = (ULONG *)&rom->er_Reserved0c;
      *ptr = diag_addr;

      AddConfigDev(cd);

      struct DeviceNode *dn = MakeDosNode(parmPkt);
      D(("got dos node=%08lx\n", dn));
      if(dn != NULL) {
        /* now add boot node */
        ok = AddBootNode( 0, ADNF_STARTPROC, dn, cd );
        D(("add boot node=%d\n", ok));
      }
    }
    CloseLibrary(ExpansionBase);
  }
  return ok;
}
