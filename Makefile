GCCFLAGS=-ffreestanding -m32 -g -c -masm=intel
NASMFLAGS=-f elf
SRCFILES=$(shell ls *.{asm,c} | grep -P "[^(boot|zeroes.asm)]")

tmp=$(SRCFILES:%.c=out/%.o)
OUTFILES=$(tmp:%.asm=out/%.o)

qemu: OS.bin
	qemu-system-x86_64 -drive format=raw,file="$<",index=0,if=floppy -m 128M

out/%.o: %.c
	gcc $(GCCFLAGS) -c $< -o $@
out/%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

boot.bin: boot.asm full_kernel.bin
	kernel_size=$(shell bc <<< "$$(du $< | cut -f1) / 512 + 1")
	sed -E boot.asm -e "s/(KERNEL_SIZE equ )0/\1$kernel_size" > out/boot.asm
	nasm "out/boot.asm" -f bin -o boot.bin

full_kernel.bin: $(OUTFILES)
	ld -o "full_kernel.bin" -Ttext 0x1000 $^ --oformat binary

everything.bin: boot.bin
	cat "boot.bin" "full_kernel.bin" > "everything.bin"

zeroes.bin:
	nasm zeroes.asm -f bin -o zeroes.bin

OS.bin: everything.bin zeroes.bin
	cat everything.bin zeroes.bin > "OS.bin"

clean:
	rm *.{bin,o} || true
	rm out/*.{bin,o} || true

.PHONY: clean qemu
