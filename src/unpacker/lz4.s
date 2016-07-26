* lz4 decompressor
* from: http://www.atari-forum.com/viewtopic.php?t=26825
* author: Anima
* patched by cnvogelg to be rommable
* assumes pre-processed lz4 streams without frame header

* a0 = packed data
* a1 = output data

    xref     _lz4_unpack

_lz4_unpack:
    movem.l   d0-d4/a0-a4,-(sp)

    addq.l    #7,a0  ; skip lz4 frame header
    moveq     #0,d2
    move.w    #$f,d4

    move.l    sp,a2 ; temp storage for numbers

next_block:
    ; read block size
    move.b    (a0)+,-(a2)
    move.b    (a0)+,-(a2)
    move.b    (a0)+,-(a2)
    move.b    (a0)+,-(a2)
    move.l    (a2)+,d3
    ; neg is direct copy
    bmi       copy_uncompressed_block
    ; null block is end
    beq       end_of_compressed_data

    ; calc end address of compressed block -> a4
    lea       (a0,d3.l),a4

next_token:
    moveq     #0,d0
    move.b    (a0)+,d0

    ; literal length
    move.w    d0,d1
    lsr.w     #4,d1
    beq.s     match_data_only

    ; 15? more length bytes follow
    cmp.w     d4,d1
    beq.s     additional_literal_length

    ; copy up to 14 direct
    subq.w    #1,d1

short_literal_copy_loop:
    move.b    (a0)+,(a1)+

    dbra      d1,short_literal_copy_loop

    bra.s     process_match_data

additional_literal_length:
    move.b    (a0)+,d2
    add.w     d2,d1
    ; last is != 0xff
    not.b     d2
    beq.s     additional_literal_length

    ; d1=lit length
    move.w    d1,d3
    lsr.w     #4,d1
    and.w     d4,d3
    add.w     d3,d3
    neg.w     d3
    jmp       literal_copy_start(pc,d3.w)

long_literal_copy_loop:
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
    move.b    (a0)+,(a1)+
literal_copy_start:
    dbra      d1,long_literal_copy_loop

process_match_data:
    ; end of block reached?
    cmpa.l    a4,a0
    beq.s     next_block

match_data_only:
    ; match length
    moveq     #0,d3
    move.b    (a0)+,-(a2)
    move.b    (a0)+,-(a2)
    move.w    (a2)+,d3

    ; calc source ptr in output buffer -> a3
    neg.l     d3
    lea       (a1,d3.l),a3

    and.w     d4,d0
    cmp.w     d4,d0
    beq.s     additional_match_length

    ; 4+short len copy
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
short_match_copy_loop:
    move.b    (a3)+,(a1)+

    dbra      d0,short_match_copy_loop

    bra.s     next_token

additional_match_length:
    move.b    (a0)+,d2
    add.w     d2,d0
    not.b     d2
    beq.s     additional_match_length

    move.w    d0,d3
    lsr.w     #4,d0
    and.w     d4,d3
    add.w     d3,d3
    neg.w     d3
    jmp       match_copy_start(pc,d3.w)

long_match_copy_loop:
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
match_copy_start:
    dbra      d0,long_match_copy_loop

    ; additional 4 ops
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+
    move.b    (a3)+,(a1)+

    bra       next_token

copy_uncompressed_block:
    andi.l    #$7fffffff,d3
block_copy_loop:
    move.b    (a0)+,(a1)+

    subq.l    #1,d3
    bne.s     block_copy_loop

    bra       next_block

end_of_compressed_data:
    movem.l   (sp)+,d0-d4/a0-a4
    rts
