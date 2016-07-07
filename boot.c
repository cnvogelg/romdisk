#define __NOLIBBASE__

#include <proto/exec.h>
#include <proto/expansion.h>

#include "debug.h"
#include "device.h"

static char execName[] = "romdisk.device";
static char dosName[] = "rom0";

ULONG parmPkt[] = {
  (ULONG)dosName,
  (ULONG)execName,
  0, /* unit */
  0, /* open flags */

  /* env block */
  16,                 /* size of table */
  512>>2,             /* # longwords in a block */
  0,                  /* sector origin -- unused */
  2,                  /* number of surfaces */
  1,                  /* secs per logical block -- leave as 1 */
  11,                 /* blocks per track */
  2,                  /* reserved blocks -- 2 boot blocks */
  0,                  /* ?? -- unused */
  0,                  /* interleave */
  0,                  /* lower cylinder */
  79,                 /* upper cylinder */
  5,                  /* number of buffers */
  MEMF_PUBLIC,        /* type of memory for buffers */
  (~0 >> 1),          /* largest transfer size (largest signed #) */
  ~1,                 /* addmask */
  0,                  /* boot priority */
  0x444f5300          /* dostype: 'DOS\0' */
};

BOOL boot_init(struct DevBase *base)
{
  BOOL ok = FALSE;

  base->expansionBase = (struct Library *)OpenLibrary("expansion.library", 36);
  if(base->expansionBase != NULL) {
    struct ConfigDev *cd = AllocConfigDev();
    D(("got expansion. config dev=%08lx\n", cd));
    if(cd != NULL) {
      /* fill faked config dev */
      cd->cd_Flags = 0;
      cd->cd_BoardAddr = (APTR)0xe00000;
      cd->cd_BoardSize = 0x080000;
      cd->cd_Driver = (APTR)base;
      struct ExpansionRom *rom = &cd->cd_Rom;
      rom->er_Type = 1;
      rom->er_Product = 42;
      rom->er_Manufacturer = 2011; /* hack id */
      rom->er_SerialNumber = 1;

      AddConfigDev(cd);

      struct DeviceNode *dn = MakeDosNode(parmPkt);
      D(("got dos node=%08lx\n", dn));
      if(dn != NULL) {
        /* now add boot node */
        ok = AddBootNode( 0, 0, dn, cd );
        D(("add boot node=%d\n", ok));
      }
    }
    CloseLibrary(base->expansionBase);
  }
  return ok;
}
