GCCFLAGS=-ffreestanding -m32 -g -c -masm=intel
NASMFLAGS=-f elf
SRCFILES=$(shell ls *.{asm,c} fs/*.c | grep -P "^(?!(boot|zeroes|kernel_entry)\.asm|test_usermode\.c).*$$")

tmp=$(SRCFILES:%.c=out/%.o)
OUTFILES=$(tmp:%.asm=out/%.o)

qemu: OS.bin testdisk.img
	qemu-system-x86_64 -drive format=raw,file="$<",index=0,if=floppy -m 128M -chardev file,id=klog,path=./kernel.log -serial chardev:klog -drive file=./testdisk.img,format=raw,index=0

out/%.o: %.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@
out/%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

boot.bin: full_kernel.bin boot.asm
	@echo Kernel size: $(shell bc <<< "$$(du -b $< | cut -f1) / 512 + 1")
	export kernel_size=$(shell bc <<< "$$(du -b $< | cut -f1) / 512 + 1"); \
	sed -E boot.asm -e "s/(KERNEL_SIZE equ )0/\1$$kernel_size/" > out/boot.asm
	nasm "out/boot.asm" -f bin -o boot.bin

full_kernel.bin: out/kernel_entry.o $(OUTFILES) # WRONG!! kernel_entry.o must be the first in this list!1
	i386-elf-ld -o "full_kernel.bin" -Ttext 0x8000 $^ --oformat binary

everything.bin: boot.bin
	cat "boot.bin" "full_kernel.bin" > "everything.bin"

zeroes.bin:
	nasm zeroes.asm -f bin -o zeroes.bin

OS.bin: everything.bin zeroes.bin
	cat everything.bin zeroes.bin > "OS.bin"

testdisk.img: out/test_usermode.o out/test_usermode2.o
	yes | mkfs.ext2 ./testdisk.img
	sudo mount ./testdisk.img tmp
	echo "Hello World" | sudo tee tmp/hello
	i386-elf-ld -o out/test -Ttext 0xA00000 out/test_usermode.o --oformat binary
	i386-elf-ld -o out/test2 -Ttext 0xA00000 out/test_usermode2.o --oformat binary
	sudo mv out/test{,2} tmp/
	sudo umount tmp

clean:
	rm *.{bin,o} || true
	rm out/*.{bin,o} || true

.PHONY: clean qemu
