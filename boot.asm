[org 0x7c00]

hello:
  db "Hello World", 0
furry:
  db "UWU", 0

mov bp, 0x8000
mov sp, bp

mov ax, 160
call prtint

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

prtint:
  mov dl, 10
  mov dh, 48
  mov ch, 0
  push 0
prtint_loop:
  div dl
  mov cl, ah
  or cl, dh
  push cx
  
  ; nothing to divide anymore
  cmp al, 0
  je prtint_endloop

  mov ah, 0
  jmp prtint_loop
prtint_endloop:
  mov bx, sp
  call strprt
  ret

times 510-($-$$) db 0
db 0x55, 0xaa
