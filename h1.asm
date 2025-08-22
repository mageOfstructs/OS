; h1.asm
; Prints the alphabet in alternating cases
; Author: mageOfStructs
; License: AGPL
mov ah, 0x0e
mov al, 'A'
mov bl, 32 ; bitmask
loop:
  int 0x10
  inc al
  cmp al, 'z'
  jg endloop
  xor al, bl
  jmp loop
endloop:

; jmp $

times 510-($-$$) db 0
db 0x55, 0xaa
