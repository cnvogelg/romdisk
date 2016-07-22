#ifndef COMPILER_H
#define COMPILER_H

/* compiler specific switches */
#ifdef __VBCC__
#define REG(r,t) __reg( #r ) t
#define SAVEDS __saveds
#define ASM
#else
#error unsupported compiler
#endif /* VBCC */

#endif /* COMPILER_H */
