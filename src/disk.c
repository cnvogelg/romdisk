#include <string.h>

#include <proto/exec.h>
#include <devices/trackdisk.h>

#include "compiler.h"
#include "debug.h"
#include "mydev.h"
#include "disk.h"
#include "unpack.h"

#define EMPTY_PACK 0xffffffff

extern ULONG theEnd;

static struct DiskHeader *disk_find_header(struct DevBase *base)
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

static BOOL disk_get_unpacked_data(struct DevBase *base, ULONG pack_id)
{
  /* already unpacked? */
  if(pack_id == base->curPackId) {
    D(("  pack: #%ld re-used\n", pack_id));
    return TRUE;
  }

  /* retrieve offset for pack from pack header */
  struct PackHeader *ph = base->packHeader;
  ULONG offset = ph->offsets[pack_id];

  /* empty pack? */
  if(offset == EMPTY_PACK) {
    D(("  pack: #%ld empty\n", pack_id));
    base->curBuffer = NULL; /* NULL marks empty pack */
    base->curPackId = pack_id;
  }
  /* packed data */
  else {
    UBYTE *pack_data = base->diskData + offset;
    base->curBuffer = base->unpackFunc(pack_data, base->unpackBuffer, ph->pack_size);
    D(("  pack: #%ld -> @%08lx -> buf=%08lx\n", pack_id, pack_data, base->curBuffer));
    if(base->curBuffer != NULL) {
      base->curPackId = pack_id;
      return TRUE;
    } else {
      base->curPackId = -1;
      return FALSE;
    }
  }
}

static void disk_raw_read(struct IOStdReq *ior, struct DevBase *base)
{
  ULONG offset = ior->io_Offset;
  ULONG length = ior->io_Length;
  APTR  buffer = ior->io_Data;

  if((offset + length) > base->diskHeader->disk_size) {
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

static void disk_pack_read(struct IOStdReq *ior, struct DevBase *base)
{
  ULONG offset = ior->io_Offset;
  ULONG length = ior->io_Length;
  UBYTE *buffer = (UBYTE *)ior->io_Data;

  if((offset + length) > base->diskHeader->disk_size) {
    ior->io_Actual = 0;
    ior->io_Error = TDERR_NotSpecified;
    D(("read out of range off=%08lx len=%08lx\n", offset, length));
  } else {
    struct PackHeader *packHeader = base->packHeader;
    ULONG pack_size = packHeader->pack_size;
    ULONG pack_id = offset / pack_size;
    ULONG pack_off = offset % pack_size;
    /* read until length reached */
    ULONG done = 0;
    BOOL last = FALSE;
    while(1) {
      ULONG l;
      /* read command crosses pack boundaries -> multiple unpacks required */
      if((pack_off + length) > pack_size) {
        l = pack_size - pack_off;
      }
      /* read fits in this pack */
      else {
        l = length;
        last = TRUE;
      }
      /* get unpacked buffer for pack_id */
      BOOL ok = disk_get_unpacked_data(base, pack_id);
      if(!ok) {
        ior->io_Error = TDERR_NotSpecified;
        D(("READ ERROR: no buffer!\n"));
        break;
      }
      /* copy/clear fragment of this pack */
      UBYTE *pack_buffer = base->curBuffer;
      if(pack_buffer != NULL) {
        UBYTE *data = pack_buffer + pack_off;
        D(("  read @%08lx: data @%08lx + %08lx with %08lx bytes\n",
           buffer, pack_buffer, pack_off, l));
        CopyMemQuick(data, buffer, l);
      } else {
        /* clear */
        memset(buffer, 0, l);
      }
      /* last pack? */
      done += l;
      if(last) {
        break;
      }
      /* next pack iteration */
      pack_id++;
      pack_off=0;
      length -= l;
      buffer += l;
    }
    /* done */
    ior->io_Actual = done;
    D(("  done %08lx\n", done));
  }
}

BOOL disk_setup(struct DevBase *base)
{
  base->diskHeader = disk_find_header(base);
  if(base->diskHeader == NULL) {
    return FALSE;
  }

  /* setup format */
  UWORD format = base->diskHeader->format;
  if(format == ROMDISK_FORMAT_RAW) {
    D(("raw format\n"));
    base->diskData = (UBYTE *)(base->diskHeader + 1);
    base->readFunc = disk_raw_read;
    base->packHeader = NULL;
  }
  else if(format == ROMDISK_FORMAT_PACK) {
    /* expect pack header after disk header */
    struct PackHeader *packHeader = (struct PackHeader *)(base->diskHeader + 1);
    base->packHeader = packHeader;
    base->readFunc = disk_pack_read;

    /* check tag */
    ULONG pack_tag = packHeader->tag;
    if(pack_tag != ROMDISK_PACK_TAG) {
      D(("no pack header tag: %08lx\n", pack_tag));
      return FALSE;
    }
    /* check packer */
    ULONG packer = packHeader->packer;
    if(packer == ROMDISK_PACK_NOP) {
      base->unpackFunc = unpack_nop;
      D(("no packer\n"));
    }
    else if(packer == ROMDISK_PACK_RNC) {
      base->unpackFunc = unpack_rnc;
      D(("RNC format\n"));
    }
    else if(packer == ROMDISK_PACK_DFLT) {
      base->unpackFunc = unpack_dflt;
      D(("DFLT format\n"));
    }
    else {
      D(("unknown packer: %08lx\n", packer));
      return FALSE;
    }

    /* calc disk base */
    UBYTE *base_ptr = (UBYTE *)(packHeader + 1);
    base_ptr += (packHeader->num_packs - 1) * sizeof(ULONG);
    base->diskData = base_ptr;
    D(("pack disk data: %08lx\n", base_ptr));
  }
  else {
    D(("unknown format: %08x\n", (ULONG)format));
    return FALSE;
  }
  return TRUE;
}

BOOL disk_open(struct DevBase *base)
{
  base->curPackId = -1;

  /* alloc pack buffer */
  struct PackHeader *ph = base->packHeader;
  if((ph != NULL) && (ph->tag != ROMDISK_PACK_NOP)) {
    base->unpackBuffer = (BYTE *)AllocMem(ph->pack_size, MEMF_PUBLIC);
    D(("unpackBuffer=%08lx size=%08lx\n", base->unpackBuffer, ph->pack_size));
    if(base->unpackBuffer == NULL) {
      return FALSE;
    }
  } else {
    base->unpackBuffer = NULL;
  }

  return TRUE;
}

void disk_close(struct DevBase *base)
{
  /* free pack buffer */
  struct PackHeader *ph = base->packHeader;
  if(base->unpackBuffer != NULL) {
    FreeMem(base->unpackBuffer, ph->pack_size);
  }
}
