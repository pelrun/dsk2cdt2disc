org $9000

kl_log_ext equ $bcd1
DEAFBCA1 equ 1
SAFEBCA1 equ 0
DARKBCA1 equ 0

start:
  ld hl,wkspce
  ld bc,jumptable
  jp kl_log_ext

wkspce:
  db 00,00,00,00

jumptable:
  dw rsxs
  jp format
  jp write_sect
  jp callbca1
  jp deexo

rsxs:
  db "FORMA",'T'+$80
  db "WRITESCT",'R'+$80
  db "CALLBCA",'1'+$80
  db "DEEX",'O'+$80
  db 0

format:
  cp $02
  ret nz

  ld l,(ix+$00)
  ld h,(ix+$01)
  ld d,(ix+$02)
  ld e,(ix+$03)
  inc  hl
  ld c,(hl)
  inc hl
  ld b,(hl)
  push bc
  pop hl
  rst $18
  dw bios_format
  ret

write_sect:
  cp $03
  ret nz

  ld l,(ix+$00)
  ld h,(ix+$01)
  ld c,(ix+$02)
  ld d,(ix+$04)
  ld e,(ix+$05)
  rst $18
  dw bios_writesect
  ret

bios_format:
  dw $c042
  db $07

bios_writesect:
  dw $c03f
  db $07

; from www.cpcwiki.eu/forum/programming/callbca1-firmware-compatible-tape-loader/
callbca1:
 cp $03
 ret nz

 ld a,(ix+$02)
 ld e,(ix+$04)
 ld d,(ix+$05)
 ld l,(ix+$06)
 ld h,(ix+$07)
 call callbca1impl
 ld l,(ix+$00)
 ld h,(ix+$01)
 ld a,0
 adc a,0
 ld (hl),a
 inc hl
 ld (hl),a
 ei
 ret

callbca1impl:
 di ; HL=^ADDR,DE=SIZE,A=ID; CF=OK,IX=^LAST.ADDR,ABCDEHL[HL'IY]!

 .if DEAFBCA1
  ld bc,$F610
 .else
  ld bc,$F40E
  out (c),c
  ld bc,$F6D0
  out (c),c
  ld c,$10 ; 20131121 WE SAVE A BYTE ;-)
 .endif

 out (c),c
 ld b,$7F
 out (c),c

 .if DEAFBCA1
 .else
  ld bc,$F792
  out (c),c
  ld bc,$F650+8 ; ESCAPE; KEYBOARD LINE 8
  out (c),c
 .endif

 push hl
 pop ix
 push de
 exx

 .if SAFEBCA1
  ex hl,(sp)
  push iy
 .else
  pop hl
 .endif

 ld iyl,a
 xor a
 cp l
 adc a,h
 ld h,a
 exx

 .if DARKBCA1
  ld c,'F' ; CYAN/RED, AS IN THE SPECTRUM
 .else
  ld c,'S' ; BRIGHT CYAN/RED LOOK BETTER!
 .endif

; DETECT PILOT TONE AS 256 SIMILAR PULSES
callbca1init:
 ld e,0
 ld h,e
 ld l,e
 ld iyh,d ; THE NEXT OPCODES SET ITS VALUE

 .if DEAFBCA1
 .else
  ld a,$F4
  in a,(0) ; A BIT BETTER THAN "IN A,(C)"
  and $04
  jr z,callbca1exit ; ESCAPE; KEYB. BIT 2
 .endif

callbca1tone:
 call callbca1edge
 jr nc,callbca1init
 sub d ; WE NEED TO KEEP D AS SEEN ABOVE!
 add a,8 ; *!* A RELATIVELY LOW THRESHOLD..
 sub 8*2+1 ; *!* TWICE THE PAST THRESHOLD
 jr nc,callbca1init ; D IS OUT OF BOUNDS!
 ld a,d
 add a,l
 ld l,a
 adc a,h
 sub l
 ld h,a
 inc e
 jr nz,callbca1tone ; WAIT FOR 256 PULSES
 srl a
 add a,h
; adc e; 20131121; BAD IDEA, LESS PRECISE
 ld iyh,a ; THIS WILL BE OUR BIT THRESHOLD

; READ PILOT UNTIL FINDING THE SYNC.PULSE
callbca1wait:
 ld e,2
 call callbca1edge
 jr nc,callbca1init ; OVERFLOW, TRY AGAIN
 srl a
 cp d
 jr c,callbca1wait ; LONG EDGE, WAIT MORE
 dec e
 jr nz,callbca1wait+2 ; PULSE = TWO EDGES

; READ AND CHECK THE ID BYTE, STILL IN iyl
 call callbca1byte
 jr nc,callbca1init
 sub iyl
 jr nz,callbca1init ; WRONG BYTE, GO BACK
 ld iyl,a ; iyl IS NOW OUR 256-BYTE COUNTER
 ld a,c

 .if DARKBCA1
  xor $18 ; YELLOW/BLUE FROM THE SPECTRUM
 .else
  xor $19 ; BRIGHT YELLOW/BLUE ARE BETTER
 .endif

 ld c,a

; READ ONE PAGE OF BYTES AND ITS CHECKSUM
callbca1next:
 ld hl,-1 ; RESET CRC16 VALUE
callbca1page:
 call callbca1byte
 jr nc,callbca1exit ; ABORT ON READ ERROR
 exx
 ld a,1 ; THE LAST PAGE IS A SPECIAL CASE
 xor h
 jr nz,callbca1skip
 ld a,l
 dec a
 cp iyl
callbca1skip:
 exx
 jr c,callbca1skip_ ; DROP TRAILING BYTES
 ld (ix),e
 inc ix ; "DEC IX" FOR LDDR-LIKE LOADING!
callbca1skip_:
 inc iyl
 jr nz,callbca1page
 push hl ; KEEP THE CRC16 FROM CHANGING!!
 call callbca1byte
 pop hl
 jr nc,callbca1exit
 xor h
 ld h,a
 push hl
 call callbca1byte
 pop hl
 jr nc,callbca1exit
 xor l
 and h
 inc a
 jr nz,callbca1exit ; IS THE CRC16 RIGHT?
 exx
 dec h
 exx
 jr nz,callbca1next ; ARE ALL PAGES DONE?
 scf

callbca1exit:
; ld bc,$7F54
; out (c),c ; LET THE USER SET THE BORDER

 .if SAFEBCA1
  pop iy
  exx
  pop hl
  exx
 .endif
 .if DEAFBCA1
 .else
  ld bc,$F782
  out (c),c
 .endif

 ld b,$F6
 dw $71ED ; THE UNDOCUMENTED "OUT (C),0"!
; ei ; IT'S UP TO THE USER TO ENABLE INTS
 ret

callbca1byte:
 ld e,1
callbca1bits:
 call callbca1edge
 call c,callbca1edge_
 ret nc
 sbc a,d ; 20140115; BETTER AVERAGE? *SUB D
 ld d,a ;*!* WE WILL NEED THE BIT 7 BELOW
 ; COPIED...
 xor h
 jp p,$+12 ; IT'S THE CRC-16-CCITT METHOD
 ld a,h
 xor $08
 ld h,a
 ld a,l
 xor $10
 ld l,a
 scf
 adc hl,hl
; ...PASTED
 rl d ;*!* AS SEEN ABOVE WE USE THE BIT 7
 rl e
 jr nc,callbca1bits
 ld a,e
 ret

callbca1edge:
 ld d,0 ; EASIER MATHEMATICS!
callbca1edge_:
 ld a,8-1 ; DELAY (~32 NOPs)
 dec a
 jr nz,$-1
 ld b,$F5
 inc d
 ret z ; OVERFLOW, THE EDGE WAS TOO LONG!
 in a,(c)
 xor c
 and $80
 jr z,$-7
 xor c

 .if DARKBCA1
  xor $1A ; ZX SPECTRUM-LIKE DARK BORDERS
 .else
  xor $1F ; BETTER-LOOKING BRIGHT BORDERS
 .endif

 ld c,a
 ld b,$7F
 and b
 out (c),a
 ld a,iyh
 scf
 ret

;Exomizer 2 Z80 decoder
; by Metalbrain
;
; optimized by Antonio Villena and Urusergi (169 bytes)
;
; compression algorithm by Magnus Lind

;input          hl=compressed data start
;               de=uncompressed destination start
;
;               you may change exo_mapbasebits to point to any free buffer
;
;ATTENTION!
;A huge speed boost (around 14%) can be gained at the cost of only 5 bytes.
;If you want this, replace all instances of "call exo_getbit" with "srl a" followed by
;"call z,exo_getbit", and remove the first two instructions in exo_getbit routine.

deexo:          cp $02
                ret nz

                ld e,(ix+$00)
                ld d,(ix+$01)
                ld l,(ix+$02)
                ld h,(ix+$03)

                ld      iy, exo_mapbasebits+11
                ld      a, (hl)
                inc     hl
                ld      b, 52
                push    de
                cp      a
exo_initbits:   ld      c, 16
                jr      nz, exo_get4bits
                ld      ixl, c
                ld      de, 1           ;DE=b2
exo_get4bits:   call    exo_getbit      ;get one bit
                rl      c
                jr      nc, exo_get4bits
                inc     c
                push    hl
                ld      hl, 1
                ld      (iy+41), c      ;bits[i]=b1 (and opcode 41 == add hl,hl)
exo_setbit:     dec     c
                jr      nz, exo_setbit-1 ;jump to add hl,hl instruction
                ld      (iy-11), e
                ld      (iy+93), d      ;base[i]=b2
                add     hl, de
                ex      de, hl
                inc     iy
                pop     hl
                dec     ixl
                djnz    exo_initbits
                pop     de
                jr      exo_mainloop
exo_literalrun: ld      e, c            ;DE=1
exo_getbits:    dec     b
                ret     z
exo_getbits1:   call    exo_getbit
                rl      e
                rl      d
                jr      nc, exo_getbits
                ld      b, d
                ld      c, e
                pop     de
exo_literalcopy:ldir
exo_mainloop:   inc     c
                call    exo_getbit      ;literal?
                jr      c, exo_literalcopy
                ld      c, 239
exo_getindex:   call    exo_getbit
                inc     c
                jr      nc,exo_getindex
                ret     z
                push    de
                ld      d, b
                jp      p, exo_literalrun
                ld      iy, exo_mapbasebits-229
                call    exo_getpair
                push    de
                rlc     d
                jr      nz, exo_dontgo
                dec     e
                ld      bc, 512+32      ;2 bits, 48 offset
                jr      z, exo_goforit
                dec     e               ;2?
exo_dontgo:     ld      bc, 1024+16     ;4 bits, 32 offset
                jr      z, exo_goforit
                ld      de, 0
                ld      c, d            ;16 offset
exo_goforit:    call    exo_getbits1
                ld      iy, exo_mapbasebits+27
                add     iy, de
                call    exo_getpair
                pop     bc
                ex      (sp), hl
                push    hl
                sbc     hl, de
                pop     de
                ldir
                pop     hl
                jr      exo_mainloop    ;Next!

exo_getpair:    add     iy, bc
                ld      e, d
                ld      b, (iy+41)
                call    exo_getbits
                ex      de, hl
                ld      c, (iy-11)
                ld      b, (iy+93)
                add     hl, bc          ;Always clear C flag
                ex      de, hl
                ret

exo_getbit:     srl     a
                ret     nz
                ld      a, (hl)
                inc     hl
                rra
                ret

exo_mapbasebits:;defs    156             ;tables for bits, baseL, baseH
