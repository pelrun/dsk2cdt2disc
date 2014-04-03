org &9000

kl_log_ext equ &bcd1
DEAFBCA1 equ 0
SAFEBCA1 equ 0
DARKBCA1 equ 0

start
  ld hl,wkspce
  ld bc,jumptable
  jp kl_log_ext

wkspce
  db 00,00,00,00

jumptable
  dw rsxs
  jp format
  jp write_sect
  jp callbca1

rsxs
  db "FORMA","T"+&80
  db "WRITESCT","R"+&80
  db "CALLBCA","1"+&80
  db 0

format
  cp &02
  ret nz

  push hl
  push de
  push bc
  ld l,(ix+&00)
  ld h,(ix+&01)
  ld d,(ix+&02)
  ld e,(ix+&03)
  inc  hl
  ld c,(hl)
  inc hl
  ld b,(hl)
  push bc
  pop hl
  rst &18
  dw bios_format
  pop bc
  pop de
  pop hl
  ret

write_sect
  cp &03
  ret nz

  push hl
  push de
  push bc
  ld l,(ix+&00)
  ld h,(ix+&01)
  ld c,(ix+&02)
  ld d,(ix+&04)
  ld e,(ix+&05)
  rst &18
  dw bios_writesect
  pop bc
  pop de
  pop hl
  ret

bios_format
  dw &c042
  db &07

bios_writesect
  dw &c03f
  db &07

callbca1 ; from www.cpcwiki.eu/forum/programming/callbca1-firmware-compatible-tape-loader/
 cp &03
 ret nz

 ld a,(ix+&00)
 ld e,(ix+&02)
 ld d,(ix+&03)
 ld l,(ix+&04)
 ld h,(ix+&05)

 di ; HL=^ADDR,DE=SIZE,A=ID; CF=OK,IX=^LAST.ADDR,ABCDEHL[HL'IY]!

 if DEAFBCA1
  ld bc,&F610
 else
  ld bc,&F40E
  out (c),c
  ld bc,&F6D0
  out (c),c
  ld c,&10 ; 20131121 WE SAVE A BYTE ;-)
 endif

 out (c),c
 ld b,&7F
 out (c),c

 if DEAFBCA1
 else
  ld bc,&F792
  out (c),c
  ld bc,&F650+8 ; ESCAPE; KEYBOARD LINE 8
  out (c),c
 endif

 push hl
 pop ix
 push de
 exx

 if SAFEBCA1
  ex hl,(sp)
  push iy
 else
  pop hl
 endif

 ld iyl,a
 xor a
 cp l
 adc h
 ld h,a
 exx

 if DARKBCA1
  ld c,"F" ; CYAN/RED, AS IN THE SPECTRUM
 else
  ld c,"S" ; BRIGHT CYAN/RED LOOK BETTER!
 endif

; DETECT PILOT TONE AS 256 SIMILAR PULSES
callbca1init ld e,0
 ld h,e
 ld l,e
 ld iyh,d ; THE NEXT OPCODES SET ITS VALUE

 if DEAFBCA1
 else
  ld a,&F4
  in a,(0) ; A BIT BETTER THAN "IN A,(C)"
  and &04
  jr z,callbca1exit ; ESCAPE; KEYB. BIT 2
 endif

callbca1tone call callbca1edge
 jr nc,callbca1init
 sub d ; WE NEED TO KEEP D AS SEEN ABOVE!
 add 8 ; *!* A RELATIVELY LOW THRESHOLD..
 sub 8*2+1 ; *!* TWICE THE PAST THRESHOLD
 jr nc,callbca1init ; D IS OUT OF BOUNDS!
 ld a,d
 add l
 ld l,a
 adc h
 sub l
 ld h,a
 inc e
 jr nz,callbca1tone ; WAIT FOR 256 PULSES
 srl a
 add h
; adc e; 20131121; BAD IDEA, LESS PRECISE
 ld iyh,a ; THIS WILL BE OUR BIT THRESHOLD

; READ PILOT UNTIL FINDING THE SYNC.PULSE
callbca1wait ld e,2
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

 if DARKBCA1
  xor &18 ; YELLOW/BLUE FROM THE SPECTRUM
 else
  xor &19 ; BRIGHT YELLOW/BLUE ARE BETTER
 endif

 ld c,a

; READ ONE PAGE OF BYTES AND ITS CHECKSUM
callbca1next ld hl,-1 ; RESET CRC16 VALUE
callbca1page call callbca1byte
 jr nc,callbca1exit ; ABORT ON READ ERROR
 exx
 ld a,1 ; THE LAST PAGE IS A SPECIAL CASE
 xor h
 jr nz,callbca1skip
 ld a,l
 dec a
 cp iyl
callbca1skip exx
 jr c,callbca1skip_ ; DROP TRAILING BYTES
 ld (ix),e
 inc ix ; "DEC IX" FOR LDDR-LIKE LOADING!
callbca1skip_ inc iyl
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

callbca1exit ; ld bc,&7F54
; out (c),c ; LET THE USER SET THE BORDER

 if SAFEBCA1
  pop iy
  exx
  pop hl
  exx
 endif
 if DEAFBCA1
 else
  ld bc,&F782
  out (c),c
 endif

 ld b,&F6
 dw &71ED ; THE UNDOCUMENTED "OUT (C),0"!
; ei ; IT'S UP TO THE USER TO ENABLE INTS
 ret

callbca1byte ld e,1
callbca1bits call callbca1edge
 call c,callbca1edge_
 ret nc
 sbc d ; 20140115; BETTER AVERAGE? *SUB D
 ld d,a ;*!* WE WILL NEED THE BIT 7 BELOW
 ; COPIED...
 xor h
 jp p,$+12 ; IT'S THE CRC-16-CCITT METHOD
 ld a,h
 xor &08
 ld h,a
 ld a,l
 xor &10
 ld l,a
 scf
 adc hl,hl
; ...PASTED
 rl d ;*!* AS SEEN ABOVE WE USE THE BIT 7
 rl e
 jr nc,callbca1bits
 ld a,e
 ret

callbca1edge ld d,0 ; EASIER MATHEMATICS!
callbca1edge_ ld a,8-1 ; DELAY (~32 NOPs)
 dec a
 jr nz,$-1
 ld b,&F5
 inc d
 ret z ; OVERFLOW, THE EDGE WAS TOO LONG!
 in a,(c)
 xor c
 and &80
 jr z,$-7
 xor c

 if DARKBCA1
  xor &1A ; ZX SPECTRUM-LIKE DARK BORDERS
 else
  xor &1F ; BETTER-LOOKING BRIGHT BORDERS
 endif

 ld c,a
 ld b,&7F
 and b
 out (c),a
 ld a,iyh
 scf
 ret
