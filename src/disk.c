#include <proto/exec.h>
#include <devices/trackdisk.h>

#include <SDI/SDI_compiler.h>

#include "debug.h"
#include "mydev.h"
#include "disk.h"

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

/* asm function */
extern ASM ULONG unpack_rnc1(REG(a0, UBYTE *packed_data),
                             REG(a1, UBYTE *out_data));

static ULONG unpack_rnc(UBYTE *packed_data, UBYTE *out_data)
{
  return unpack_rnc1(packed_data, out_data);
}

static UBYTE *disk_get_pack_data(struct DevBase *base, ULONG pack_id)
{
  /* already unpacked? */
  if(pack_id == base->curPackId) {
    D(("reuse pack: #%ld\n", pack_id));
    return base->unpackBuffer;
  }

  struct PackHeader *ph = base->packHeader;
  ULONG offset = ph->offsets[pack_id];
  UBYTE *pack_data = base->diskData + offset;

  ULONG unpack_size = base->unpackFunc(pack_data, base->unpackBuffer);
  D(("unpack: #%ld -> @%08lx -> got %08lx\n", pack_id, pack_data, unpack_size));

  if(unpack_size == ph->pack_size) {
    base->curPackId = pack_id;
    return base->unpackBuffer;
  } else {
    base->curPackId = -1;
    return NULL;
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
  APTR  buffer = ior->io_Data;

  if((offset + length) > base->diskHeader->disk_size) {
    ior->io_Actual = 0;
    ior->io_Error = TDERR_NotSpecified;
    D(("read out of range off=%08lx len=%08lx\n", offset, length));
  } else {
    struct PackHeader *packHeader = base->packHeader;
    ULONG pack_size = packHeader->pack_size;
    ULONG pack_id = offset / pack_size;
    ULONG pack_off = offset % pack_size;
    /* currently read op has to fit inside pack */
    if((pack_off + length) > pack_size) {
      ior->io_Actual = 0;
      ior->io_Error = TDERR_NotSpecified;
      D(("pack read out of range: pack_off=%08lx len=%08lx\n", pack_off, length));
    }
    else {
      UBYTE *pack_buffer = disk_get_pack_data(base, pack_id);
      if(pack_buffer == NULL) {
        ior->io_Actual = 0;
        ior->io_Error = TDERR_NotSpecified;
      } else {
        ior->io_Actual = length;
        UBYTE *data = pack_buffer + pack_off;
        D(("read data @%08lx + %08lx with %08lx bytes\n", pack_buffer, pack_off, length));
        CopyMemQuick(data, buffer, length);
      }
    }
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
    if(packer == ROMDISK_PACK_RNC) {
      base->unpackFunc = unpack_rnc;
      D(("RNC format\n"));
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
  struct PackHeader *ph = base->packHeader;
  if(ph != NULL) {
    base->unpackBuffer = (BYTE *)AllocMem(ph->pack_size, 0);
    base->curPackId = -1;
    D(("unpackBuffer=%08lx\n", base->unpackBuffer));
    if(base->unpackBuffer == NULL) {
      return FALSE;
    }
  }
  return TRUE;
}

void disk_close(struct DevBase *base)
{
  struct PackHeader *ph = base->packHeader;
  if(base->unpackBuffer != NULL) {
    FreeMem(base->unpackBuffer, ph->pack_size);
  }
}
