MEMSIZE := 0x200
TARGET  := prem
CC      := sdcc
AS	:= sdasz80
CFLAGS  :=
LFLAGS  := -Wl-u
MFLAGS  := -mz80 --code-loc 0x001C \
           --data-loc 0x0000 --vc --verbose

all: ${TARGET}.bin

crt0.rel: crt0.s
	$(AS) -plosgffw crt0.rel crt0.s

putchar.rel: putchar.s
	$(AS) -plosgffw putchar.rel putchar.s

${TARGET}.rel: ${TARGET}.c
	$(CC) -mz80 -c ${TARGET}.c

${TARGET}.ihx: ${TARGET}.rel crt0.rel putchar.rel
	$(CC) $(MFLAGS) $(LFLAGS) --no-std-crt0 crt0.rel \
	${TARGET}.rel putchar.rel -o ${TARGET}.ihx

${TARGET}.bin: ${TARGET}.ihx
	makebin -p < ${TARGET}.ihx > ${TARGET}.bin
	objcopy -Iihex -Obinary --gap-fill 0x00 --pad-to \
	$(MEMSIZE) ${TARGET}.ihx ${TARGET}_padded.bin

disasm: ${TARGET}.bin
	z80dasm -t -g 0x0 -l ${TARGET}.bin

dump: ${TARGET}.bin
	xxd -c 16 -i ${TARGET}.bin

paddump: ${TARGET}.bin
	xxd -c 16 -i ${TARGET}_padded.bin

clean:
	rm -f  ${TARGET}.asm ${TARGET}.cdb ${TARGET}.rel \
	${TARGET}.hex ${TARGET}.ihx ${TARGET}.lst ${TARGET}.map \
	${TARGET}.o ${TARGET}.rst ${TARGET}.sym ${TARGET}.lnk \
	${TARGET}.lib ${TARGET}.bin ${TARGET}.mem ${TARGET}.lk \
	${TARGET}.noi
	rm -f *.lst *.rel *.rst *.sym *.bin
