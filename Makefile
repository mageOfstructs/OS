SHARED_FLAGS=-g
GCCFLAGS=$(SHARED_FLAGS) -ffreestanding -m32 -c -masm=intel
NASMFLAGS=$(SHARED_FLAGS) -f elf32
SRCFILES=$(shell ls *.{asm,c,rs} fs/*.c stdlib/printf.c | grep -P "^(?!(boot|zeroes|kernel_entry)\.asm|test_usermode\.c|user_putchar\.c).*$$")

STDLIB_SRCFILES=$(wildcard stdlib/*.c)
STDLIB_OUTFILES=$(STDLIB_SRCFILES:%.c=out/%.o)

tmp=$(SRCFILES:%.c=out/%.o)
tmp2=$(tmp:%.rs=out/%.o)
OUTFILES=$(tmp2:%.asm=out/%.o)

qemu_limine: OS_limine.iso testdisk.img
	./run_qemu.sh "$<"

debug: OS_limine.iso testdisk.img
	./run_qemu.sh "$<" -S -s

qemu: OS.iso testdisk.img
	qemu-system-i386 -cdrom "$<" -m 1G -chardev file,id=klog,path=./kernel.log -serial chardev:klog -drive file=./testdisk.img,format=raw,index=0

OS.iso: full_kernel.bin isodir/boot/grub/grub.cfg
	if ! grub-file --is-x86-multiboot2 full_kernel.bin; then echo "No multiboot2 header!" && exit 1; fi
	cp full_kernel.bin isodir/boot/
	grub-mkrescue -o OS.iso isodir

OS_limine.iso: full_kernel.bin limine_disk/limine.cfg limine/limine-deploy limine_disk/limine-cd.bin limine_disk/limine-cd-efi.bin
	mkdir -p limine_disk/boot
	cp $< limine_disk/boot
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot \
    -boot-load-size 4 -boot-info-table --efi-boot \
    limine-cd-efi.bin -efi-boot-part --efi-boot-image \
    --protective-msdos-label limine_disk -o $@
	./limine/limine-deploy $@

limine/limine-deploy:
	tmp="$@" && if ! [ -a "${tmp%/*}" ]; then \
	  git clone https://github.com/limine-bootloader/limine.git --branch=v3.0-branch-binary --depth=1; \
	fi
	$(MAKE) -C limine limine-deploy

limine_disk/limine-cd.bin: limine/limine-deploy
	cp limine/limine-cd.bin $@
limine_disk/limine-cd-efi.bin: limine/limine-deploy
	cp limine/limine-cd-efi.bin $@

out/%.o: %.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@
out/stdlib/%.o: stdlib/%.c
	i386-elf-gcc $(GCCFLAGS) -c $< -o $@

out/%.o: %.asm
	nasm $< $(NASMFLAGS) -o $@

out/%.o: %.rs
	rustc $< --target x86_64-unknown-none --emit obj -o $@

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
