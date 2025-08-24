mov ah, 0x0e
mov al, 'A'
loop:
  int 0x10
  inc al
  cmp al, 'Z'
  jg endloop
  jmp loop
endloop:

; jmp $

times 510-($-$$) db 0
db 0x55, 0xaa
