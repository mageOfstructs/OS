qemu: OS.bin
	qemu-system-x86_64 -drive format=raw,file="OS.bin",index=0,if=floppy -m 128M
	
kernel.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "kernel.c" -o "kernel.o" -masm=intel
printf.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "printf.c" -o "printf.o"
cursor.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "cursor.c" -o "cursor.o" -masm=intel
io.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "io.c" -o "io.o" -masm=intel
pic.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "pic.c" -o "pic.o" -masm=intel
init_idt.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "init_idt.c" -o "init_idt.o" -masm=intel
isr.o:
	i386-elf-gcc -ffreestanding -m32 -g -c "isr.c" -o "isr.o" -masm=intel

kernel_entry.o:
	nasm "kernel_entry.asm" -f elf -o "kernel_entry.o"
idt.o:
	nasm "idt.asm" -f elf -o "idt.o"
raw_isr.o:
	nasm "raw_isr.asm" -f elf -o "raw_isr.o"

utils.o:
	nasm "utils.asm" -f elf -o "utils.o"

boot.bin:
	nasm "boot.asm" -f bin -o boot.bin

full_kernel.bin: kernel_entry.o kernel.o printf.o cursor.o pic.o io.o idt.o init_idt.o isr.o raw_isr.o
	i386-elf-ld -o "full_kernel.bin" -Ttext 0x1000 $^ --oformat binary

everything.bin: boot.bin full_kernel.bin
	cat "boot.bin" "full_kernel.bin" > "everything.bin"

zeroes.bin:
	nasm zeroes.asm -f bin -o zeroes.bin

OS.bin: everything.bin zeroes.bin
	cat everything.bin zeroes.bin > "OS.bin"

clean:
	rm *.{bin,o}
