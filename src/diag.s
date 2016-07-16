  include "exec/types.i"
  include "exec/nodes.i"
  include "exec/resident.i"
  include "libraries/configvars.i"

_LVOFindResident equ -96

  xdef _myDiagArea
  xdef _theEnd

_myDiagArea:
DiagStart:
  dc.b    DAC_WORDWIDE+DAC_CONFIGTIME    ; da_Config
  dc.b    0                              ; da_Flags
  dc.w    EndCopy-DiagStart              ; da_Size
  dc.w    DiagEntry-DiagStart            ; da_DiagPoint
  dc.w    BootEntry-DiagStart            ; da_BootPoint
  dc.w    DevName-DiagStart              ; da_Name
  dc.w    0                              ; da_Reserved01
  dc.w    0                              ; da_Reserved02

DiagEntry:
  moveq #1,d0
  rts

BootEntry:
  lea     DosName(PC),a1          ; 'dos.library',0
  jsr     _LVOFindResident(a6)    ; find the DOS resident tag
  move.l  d0,a0                   ; in order to bootstrap
  move.l  RT_INIT(A0),a0          ; set vector to DOS INIT
  jsr     (a0)                    ; and initialize DOS
  rts

DevName:  dc.b  'romdisk.device',0
DosName:  dc.b  'dos.library',0

EndCopy:

  ; theEnd marks the end of this device and the begin of the disk image
  cnop 0,4
_theEnd:
  dc.l $deadbeef
