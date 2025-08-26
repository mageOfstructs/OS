GCCFLAGS=-ffreestanding -m32 -g -c -masm=intel
NASMFLAGS=-f elf
SRCFILES=$(shell ls *.{asm,c} | grep -P "[^(boot|zeroes.asm)]")

tmp=$(SRCFILES:%.c=out/%.o)
OUTFILES=$(tmp:%.asm=out/%.o)

qemu: OS.bin
	qemu-system-x86_64 -drive format=raw,file="$<",index=0,if=floppy -m 128M

out/%.o: %.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@
out/%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

boot.bin: full_kernel.bin boot.asm
	export kernel_size=$(shell bc <<< "$$(ls -lB $< | cut -f5 -d " ") / 512 + 1"); \
	sed -E boot.asm -e "s/(KERNEL_SIZE equ )0/\1$$kernel_size/" > out/boot.asm
	nasm "out/boot.asm" -f bin -o boot.bin

full_kernel.bin: $(OUTFILES)
	i386-elf-ld -o "full_kernel.bin" -Ttext 0x1000 $^ --oformat binary

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
