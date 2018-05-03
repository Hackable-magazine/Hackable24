UART0  .EQU     0x00            ; DATA IN/OUT
UART1  .EQU     0x01            ; CHECK RX
UART2  .EQU     0x02            ; INTERRUPTS
UART3  .EQU     0x03            ; LINE CONTROL
UART4  .EQU     0x04            ; MODEM CONTROL
UART5  .EQU     0x05            ; LINE STATUS
UART6  .EQU     0x06            ; MODEM STATUS
UART7  .EQU     0x07            ; SCRATCH REG.

	.module putchar
	.area   _CODE

_putchar::
	ld      hl,#2
	add     hl,sp
sendchar_s:
	in	a,(UART5)
	bit	#5,a
	jp	z,sendchar_s
	ld      a,(hl)
	out     (UART0),a
	ret
