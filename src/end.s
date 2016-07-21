  xdef _theEnd

  ; theEnd marks the end of this device and the begin of the disk image
  cnop 0,4
_theEnd:
  dc.l $deadbeef
