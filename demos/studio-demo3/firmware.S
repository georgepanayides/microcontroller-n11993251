
D:\CAB202\studio\demos\studio11_demo3\.pio\build\QUTy\firmware.elf:     file format elf32-avr


Disassembly of section .text:

00000000 <__vectors>:
   0:	0c 94 3c 00 	jmp	0x78	; 0x78 <__ctors_end>
   4:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
   8:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
   c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  10:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  14:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  18:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  1c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  20:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  24:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  28:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  2c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  30:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  34:	0c 94 50 00 	jmp	0xa0	; 0xa0 <__vector_13>
  38:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  3c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  40:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  44:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  48:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  4c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  50:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  54:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  58:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  5c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  60:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  64:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  68:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  6c:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  70:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>
  74:	0c 94 4e 00 	jmp	0x9c	; 0x9c <__bad_interrupt>

00000078 <__ctors_end>:
  78:	11 24       	eor	r1, r1
  7a:	1f be       	out	0x3f, r1	; 63
  7c:	cf ef       	ldi	r28, 0xFF	; 255
  7e:	cd bf       	out	0x3d, r28	; 61
  80:	df e3       	ldi	r29, 0x3F	; 63
  82:	de bf       	out	0x3e, r29	; 62

00000084 <__do_clear_bss>:
  84:	28 e3       	ldi	r18, 0x38	; 56
  86:	a0 e0       	ldi	r26, 0x00	; 0
  88:	b8 e3       	ldi	r27, 0x38	; 56
  8a:	01 c0       	rjmp	.+2      	; 0x8e <.do_clear_bss_start>

0000008c <.do_clear_bss_loop>:
  8c:	1d 92       	st	X+, r1

0000008e <.do_clear_bss_start>:
  8e:	a2 30       	cpi	r26, 0x02	; 2
  90:	b2 07       	cpc	r27, r18
  92:	e1 f7       	brne	.-8      	; 0x8c <.do_clear_bss_loop>
  94:	0e 94 c5 00 	call	0x18a	; 0x18a <main>
  98:	0c 94 dc 00 	jmp	0x1b8	; 0x1b8 <_exit>

0000009c <__bad_interrupt>:
  9c:	0c 94 00 00 	jmp	0	; 0x0 <__vectors>

000000a0 <__vector_13>:
  a0:	1f 92       	push	r1
  a2:	0f 92       	push	r0
  a4:	0f b6       	in	r0, 0x3f	; 63
  a6:	0f 92       	push	r0
  a8:	11 24       	eor	r1, r1
  aa:	2f 93       	push	r18
  ac:	8f 93       	push	r24
  ae:	9f 93       	push	r25
  b0:	ef 93       	push	r30
  b2:	ff 93       	push	r31
  b4:	cf 93       	push	r28
  b6:	df 93       	push	r29
  b8:	cd b7       	in	r28, 0x3d	; 61
  ba:	de b7       	in	r29, 0x3e	; 62
  bc:	80 91 00 38 	lds	r24, 0x3800	; 0x803800 <__DATA_REGION_ORIGIN__>
  c0:	90 91 01 38 	lds	r25, 0x3801	; 0x803801 <__DATA_REGION_ORIGIN__+0x1>
  c4:	01 96       	adiw	r24, 0x01	; 1
  c6:	80 93 00 38 	sts	0x3800, r24	; 0x803800 <__DATA_REGION_ORIGIN__>
  ca:	90 93 01 38 	sts	0x3801, r25	; 0x803801 <__DATA_REGION_ORIGIN__+0x1>
  ce:	80 e8       	ldi	r24, 0x80	; 128
  d0:	9a e0       	ldi	r25, 0x0A	; 10
  d2:	21 e0       	ldi	r18, 0x01	; 1
  d4:	fc 01       	movw	r30, r24
  d6:	26 83       	std	Z+6, r18	; 0x06
  d8:	df 91       	pop	r29
  da:	cf 91       	pop	r28
  dc:	ff 91       	pop	r31
  de:	ef 91       	pop	r30
  e0:	9f 91       	pop	r25
  e2:	8f 91       	pop	r24
  e4:	2f 91       	pop	r18
  e6:	0f 90       	pop	r0
  e8:	0f be       	out	0x3f, r0	; 63
  ea:	0f 90       	pop	r0
  ec:	1f 90       	pop	r1
  ee:	18 95       	reti

000000f0 <timer_init>:
  f0:	cf 93       	push	r28
  f2:	df 93       	push	r29
  f4:	cd b7       	in	r28, 0x3d	; 61
  f6:	de b7       	in	r29, 0x3e	; 62
  f8:	80 e8       	ldi	r24, 0x80	; 128
  fa:	9a e0       	ldi	r25, 0x0A	; 10
  fc:	fc 01       	movw	r30, r24
  fe:	11 82       	std	Z+1, r1	; 0x01
 100:	80 e8       	ldi	r24, 0x80	; 128
 102:	9a e0       	ldi	r25, 0x0A	; 10
 104:	20 e1       	ldi	r18, 0x10	; 16
 106:	37 e2       	ldi	r19, 0x27	; 39
 108:	fc 01       	movw	r30, r24
 10a:	24 87       	std	Z+12, r18	; 0x0c
 10c:	35 87       	std	Z+13, r19	; 0x0d
 10e:	80 e8       	ldi	r24, 0x80	; 128
 110:	9a e0       	ldi	r25, 0x0A	; 10
 112:	21 e0       	ldi	r18, 0x01	; 1
 114:	fc 01       	movw	r30, r24
 116:	25 83       	std	Z+5, r18	; 0x05
 118:	80 e8       	ldi	r24, 0x80	; 128
 11a:	9a e0       	ldi	r25, 0x0A	; 10
 11c:	21 e0       	ldi	r18, 0x01	; 1
 11e:	fc 01       	movw	r30, r24
 120:	20 83       	st	Z, r18
 122:	df 91       	pop	r29
 124:	cf 91       	pop	r28
 126:	08 95       	ret

00000128 <led_init>:
 128:	cf 93       	push	r28
 12a:	df 93       	push	r29
 12c:	cd b7       	in	r28, 0x3d	; 61
 12e:	de b7       	in	r29, 0x3e	; 62
 130:	80 e2       	ldi	r24, 0x20	; 32
 132:	94 e0       	ldi	r25, 0x04	; 4
 134:	20 e2       	ldi	r18, 0x20	; 32
 136:	fc 01       	movw	r30, r24
 138:	25 83       	std	Z+5, r18	; 0x05
 13a:	80 e2       	ldi	r24, 0x20	; 32
 13c:	94 e0       	ldi	r25, 0x04	; 4
 13e:	20 e2       	ldi	r18, 0x20	; 32
 140:	fc 01       	movw	r30, r24
 142:	21 83       	std	Z+1, r18	; 0x01
 144:	df 91       	pop	r29
 146:	cf 91       	pop	r28
 148:	08 95       	ret

0000014a <clock_init>:
 14a:	cf 93       	push	r28
 14c:	df 93       	push	r29
 14e:	cd b7       	in	r28, 0x3d	; 61
 150:	de b7       	in	r29, 0x3e	; 62
 152:	84 e3       	ldi	r24, 0x34	; 52
 154:	90 e0       	ldi	r25, 0x00	; 0
 156:	28 ed       	ldi	r18, 0xD8	; 216
 158:	fc 01       	movw	r30, r24
 15a:	20 83       	st	Z, r18
 15c:	80 e6       	ldi	r24, 0x60	; 96
 15e:	90 e0       	ldi	r25, 0x00	; 0
 160:	21 e0       	ldi	r18, 0x01	; 1
 162:	fc 01       	movw	r30, r24
 164:	21 83       	std	Z+1, r18	; 0x01
 166:	df 91       	pop	r29
 168:	cf 91       	pop	r28
 16a:	08 95       	ret

0000016c <initialisation>:
 16c:	cf 93       	push	r28
 16e:	df 93       	push	r29
 170:	cd b7       	in	r28, 0x3d	; 61
 172:	de b7       	in	r29, 0x3e	; 62
 174:	f8 94       	cli
 176:	0e 94 a5 00 	call	0x14a	; 0x14a <clock_init>
 17a:	0e 94 94 00 	call	0x128	; 0x128 <led_init>
 17e:	0e 94 78 00 	call	0xf0	; 0xf0 <timer_init>
 182:	78 94       	sei
 184:	df 91       	pop	r29
 186:	cf 91       	pop	r28
 188:	08 95       	ret

0000018a <main>:
 18a:	cf 93       	push	r28
 18c:	df 93       	push	r29
 18e:	cd b7       	in	r28, 0x3d	; 61
 190:	de b7       	in	r29, 0x3e	; 62
 192:	0e 94 b6 00 	call	0x16c	; 0x16c <initialisation>
 196:	80 91 00 38 	lds	r24, 0x3800	; 0x803800 <__DATA_REGION_ORIGIN__>
 19a:	90 91 01 38 	lds	r25, 0x3801	; 0x803801 <__DATA_REGION_ORIGIN__+0x1>
 19e:	88 3e       	cpi	r24, 0xE8	; 232
 1a0:	93 40       	sbci	r25, 0x03	; 3
 1a2:	c9 f7       	brne	.-14     	; 0x196 <main+0xc>
 1a4:	80 e2       	ldi	r24, 0x20	; 32
 1a6:	94 e0       	ldi	r25, 0x04	; 4
 1a8:	20 e2       	ldi	r18, 0x20	; 32
 1aa:	fc 01       	movw	r30, r24
 1ac:	27 83       	std	Z+7, r18	; 0x07
 1ae:	10 92 00 38 	sts	0x3800, r1	; 0x803800 <__DATA_REGION_ORIGIN__>
 1b2:	10 92 01 38 	sts	0x3801, r1	; 0x803801 <__DATA_REGION_ORIGIN__+0x1>
 1b6:	ef cf       	rjmp	.-34     	; 0x196 <main+0xc>

000001b8 <_exit>:
 1b8:	f8 94       	cli

000001ba <__stop_program>:
 1ba:	ff cf       	rjmp	.-2      	; 0x1ba <__stop_program>
