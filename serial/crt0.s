UART0  .EQU     0x00            ; DATA IN/OUT
UART1  .EQU     0x01            ; CHECK RX
UART2  .EQU     0x02            ; INTERRUPTS
UART3  .EQU     0x03            ; LINE CONTROL
UART4  .EQU     0x04            ; MODEM CONTROL
UART5  .EQU     0x05            ; LINE STATUS
UART6  .EQU     0x06            ; MODEM STATUS
UART7  .EQU     0x07            ; SCRATCH REG.

	.module crt0
	.globl	_main

	.area	_HEADER (ABS)
	;; Reset vector
	.org 	0x0
init:
	ld	sp,#0x0200

	ld	a,#0x80
	out	(UART3),a
	ld	a,#96
	out	(UART0),a
	ld	a,#0x00
	out	(UART1),a
	ld	a,#0x03
	out	(UART3),a

	call	gsinit
	call	_main

	.area   _HOME
	.area	_CODE
	.area   _INITIALIZER
	.area   _GSINIT
        .area   _GSFINAL
	.area	_DATA
	.area   _INITIALIZED
	.area   _BSEG
	.area   _BSS
	.area   _HEAP

	.area   _GSINIT
gsinit::
	ld      bc, #l__INITIALIZER
	ld      a, b
	or      a, c
	jr      Z, gsinit_next
	ld      de, #s__INITIALIZED
	ld      hl, #s__INITIALIZER
	ldir
gsinit_next:
        .area   _GSFINAL
        ret

