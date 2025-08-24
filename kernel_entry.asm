[bits 32]
[extern main]
[extern setup_idt]
call setup_idt
call main
jmp $
