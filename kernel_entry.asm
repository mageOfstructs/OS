[bits 32]
[extern main]

MH_MAGIC equ 0xE85250D6
MH_ARCH equ 0
MH_SZ equ (multiboot_header_end - multiboot_header)

section .multiboot align=8
multiboot_header:
dd MH_MAGIC                                ; header magic
dd MH_ARCH                                 ; 32 bit i386
dd MH_SZ                                   ; header length
dd -(MH_MAGIC + MH_ARCH + MH_SZ)



entry_tag:
dw 3
dw 0
dd entry_tag_end - entry_tag
dd kernel_entry
dd 0
entry_tag_end:


; end tag
dd 0
dd 8
multiboot_header_end:

section .bss align=16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

section .text
global _start
_start:
kernel_entry:

mov esp, stack_top
call main

loop:
hlt
jmp loop
