[org 0x7c00]

loop:
  mov ah, 0
  int 0x16
  mov ah, 0x0e
  int 0x10
  jmp loop
endloop:

jmp $

times 510-($-$$) db 0
db 0x55, 0xaa
