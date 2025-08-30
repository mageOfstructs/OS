[bits 32]
[extern main]
call main
loop:
hlt
jmp loop
