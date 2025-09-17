GCCFLAGS=-ffreestanding -m32 -g -c -masm=intel
NASMFLAGS=-f elf32 -g
SRCFILES=$(shell ls *.{asm,c} fs/*.c stdlib/printf.c | grep -P "^(?!(boot|zeroes|kernel_entry)\.asm|test_usermode\.c|user_putchar\.c).*$$")

STDLIB_SRCFILES=$(wildcard stdlib/*.c)
STDLIB_OUTFILES=$(STDLIB_SRCFILES:%.c=out/%.o)

tmp=$(SRCFILES:%.c=out/%.o)
OUTFILES=$(tmp:%.asm=out/%.o)

qemu: OS.iso testdisk.img
	qemu-system-i386 -cdrom "$<" -m 1G -chardev file,id=klog,path=./kernel.log -serial chardev:klog -drive file=./testdisk.img,format=raw,index=0

OS.iso: full_kernel.bin isodir/boot/grub/grub.cfg
	if ! grub-file --is-x86-multiboot2 full_kernel.bin; then echo "No multiboot2 header!" && exit 1; fi
	cp full_kernel.bin isodir/boot/
	grub-mkrescue -o OS.iso isodir

out/%.o: %.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@
out/stdlib/%.o: stdlib/%.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@

out/%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

boot.bin: full_kernel.bin boot.asm
	@echo Kernel size: $(shell bc <<< "$$(du -b $< | cut -f1) / 512 + 1")
	export kernel_size=$(shell bc <<< "$$(du -b $< | cut -f1) / 512 + 1"); \
	sed -E boot.asm -e "s/(KERNEL_SIZE equ )0/\1$$kernel_size/" > out/boot.asm
	nasm "out/boot.asm" -f bin -o boot.bin

full_kernel.bin: out/kernel_entry.o $(OUTFILES) # WRONG!! kernel_entry.o must be the first in this list!1
	i386-elf-ld -g -o "full_kernel.bin" $^

everything.bin: boot.bin
	cat "boot.bin" "full_kernel.bin" > "everything.bin"

zeroes.bin:
	nasm zeroes.asm -f bin -o zeroes.bin

OS.bin: everything.bin zeroes.bin
	cat everything.bin zeroes.bin > "OS.bin"

testdisk.img: out/test_usermode.o out/test_usermode2.o $(STDLIB_OUTFILES)
	yes | mkfs.ext2 ./testdisk.img
	sudo mount ./testdisk.img tmp
	echo "Hello World" | sudo tee tmp/hello
	i386-elf-ld -o out/test -Ttext 0xA00000 out/test_usermode.o $(STDLIB_OUTFILES) --oformat binary
	i386-elf-ld -o out/test2 -Ttext 0xA00000 out/test_usermode2.o $(STDLIB_OUTFILES) --oformat binary
	sudo mv out/test{,2} tmp/
	sudo umount tmp

clean:
	rm *.{bin,o} || true
	rm out/*.{bin,o} || true

.PHONY: clean qemu
