#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

UBYTE * unpack_nop(UBYTE *packed_data, UBYTE *out_buffer, ULONG out_size)
{
  /* packed data is not packed at all */
  return packed_data;
}

/* asm function */
extern ASM ULONG unpack_rnc1(REG(a0, UBYTE *packed_data),
                             REG(a1, UBYTE *out_data));

UBYTE * unpack_rnc(UBYTE *packed_data, UBYTE *out_buffer, ULONG out_size)
{
  ULONG *tag = (ULONG *)packed_data;
  /* is really packed with RNC1 */
  if(*tag == 0x524e4301) {
    unpack_rnc1(packed_data, out_buffer);
    return out_buffer;
  }
  /* raw track */
  else {
    return packed_data;
  }
}

extern ASM ULONG inflate(REG(a5, UBYTE *packed_data),
                         REG(a4, UBYTE *out_data));

UBYTE * unpack_dflt(UBYTE *packed_data, UBYTE *out_buffer, ULONG out_size)
{
  inflate(packed_data, out_buffer);
  return out_buffer;
}


extern ASM void lz4_unpack(REG(a0, UBYTE *packed_data),
                           REG(a1, UBYTE *out_data));

UBYTE * unpack_lz4(UBYTE *packed_data, UBYTE *out_buffer, ULONG out_size)
{
  lz4_unpack(packed_data, out_buffer);
  return out_buffer;
}
