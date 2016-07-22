  ; hack for amiga.lib's reference to _SysBase
  xdef _SysBase
_SysBase equ 4

  ; theEnd marks the end of this device and the begin of the disk image
  cnop 0,4
  xdef _theEnd
_theEnd:
  dc.l $deadbeef
