[org 0x7c00]

; setting up the stack

xor ax, ax
mov es, ax ; nasm doesn't like assigning immediate (is this the right term?) values to es
mov ds, ax
mov bp, 0x8000 ; mmmmm magick numbrssss
mov sp, bp

jmp $

; prints a NUL-terminated string
; bx: pointer to str
strprt:
  mov ah, 0x0e
loop:
  mov al, [bx]
  cmp al, 0
  je endloop
  int 0x10
  add bx,2
  jmp loop
endloop:
  ret

times 510-($-$$) db 0
db 0x55, 0xaa
