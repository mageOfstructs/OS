qemu: OS.bin
	qemu-system-x86_64 -drive format=raw,file="OS.bin",index=0,if=floppy -m 128M
	
kernel.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "kernel.c" -o "kernel.o"

kernel_entry.o:
	nasm "kernel_entry.asm" -f elf -o "kernel_entry.o"

utils.o:
	nasm "utils.asm" -f elf -o "utils.o"

boot.bin:
	nasm "boot.asm" -f bin -o boot.bin

full_kernel.bin: kernel_entry.o kernel.o utils.o
	i386-elf-ld -o "full_kernel.bin" -Ttext 0x1000 "kernel_entry.o" "kernel.o" "utils.o" --oformat binary

everything.bin: boot.bin full_kernel.bin
	cat "boot.bin" "full_kernel.bin" > "everything.bin"

zeroes.bin:
	nasm zeroes.asm -f bin -o zeroes.bin

OS.bin: everything.bin zeroes.bin
	cat everything.bin zeroes.bin > "OS.bin"

clean:
	rm *.{bin,o}
