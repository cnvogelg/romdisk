#define __NOLIBBASE__

#include <proto/exec.h>
#include <proto/expansion.h>

#include <exec/libraries.h>
#include <libraries/expansion.h>

#include "debug.h"
#include "mydev.h"
#include "disk.h"

#include <SDI/SDI_compiler.h>

static const char execName[] = "romdisk.device";

extern struct DiagArea myDiagArea;

static ULONG *create_param_pkt(struct DevBase *base, ULONG *size)
{
  *size = 21 * 4;
  ULONG *paramPkt = (ULONG *)AllocMem(*size, 0);
  if(paramPkt == NULL) {
    return NULL;
  }

  struct DiskHeader *hdr = base->diskHeader;

  paramPkt[0] = (ULONG)hdr->name;
  paramPkt[1] = (ULONG)execName;
  paramPkt[2] = 0; /* unit */
  paramPkt[3] = 0; /* open flags */
  /* env block */
  paramPkt[4] = 16;                 /* size of table */
  paramPkt[5] = 512>>2;             /* 0 # longwords in a block */
  paramPkt[6] = 0;                  /* 1 sector origin -- unused */
  paramPkt[7] = hdr->heads;         /* 2 number of surfaces */
  paramPkt[8] = 1;                  /* 3 secs per logical block -- leave as 1 */
  paramPkt[9] = hdr->sectors;       /* 4 blocks per track */
  paramPkt[10] = 2;                 /* 5 reserved blocks -- 2 boot blocks */
  paramPkt[11] = 0;                 /* 6 ?? -- unused */
  paramPkt[12] = 0;                 /* 7 interleave */
  paramPkt[13] = 0;                 /* 8 lower cylinder */
  paramPkt[14] = hdr->cylinders-1;  /* 9 upper cylinder */
  paramPkt[15] = hdr->num_buffers;  /* a number of buffers */
  paramPkt[16] = MEMF_PUBLIC;       /* b type of memory for buffers */
  paramPkt[17] = 0x7fffffff;        /* c largest transfer size (largest signed #) */
  paramPkt[18] = ~1;                /* d addmask */
  paramPkt[19] = hdr->boot_prio;    /* e boot priority */
  paramPkt[20] = hdr->dos_type;     /* f dostype: 'DOS\0' */

  return paramPkt;
}

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

      ULONG paramSize = 0;
      ULONG *paramPkt = create_param_pkt(base, &paramSize);
      D(("got param pkt=%08lx\n", paramPkt));
      if(paramPkt != NULL) {
        struct DeviceNode *dn = MakeDosNode(paramPkt);
        D(("got dos node=%08lx\n", dn));
        if(dn != NULL) {
          /* now add boot node */
          ok = AddBootNode( 0, ADNF_STARTPROC, dn, cd );
          D(("add boot node=%d\n", ok));
        }
        FreeMem(paramPkt, paramSize);
      }
    }
    CloseLibrary(ExpansionBase);
  }
  return ok;
}
