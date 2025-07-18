[org 0x7c00]

; stupid stack
mov bp, 0x8000
mov sp, bp
mov bx, sp

loop:
  mov ah, 0
  int 0x16
  
  mov ah, 0 ; remove the scancode thingy
  push ax
  
  cmp al, 0x0A ; newline
  je endloop
  
  cmp al, 0x0D ; other newline
  je endloop

  push ax
  mov ah, 0x0e
  int 0x10
  pop ax
  
  jmp loop
endloop:
  push 0
  
  push ax
  mov ah, 0x0e
  mov al, 'Z'
  int 0x10
  pop ax

  sub bx, 1
  call stupid_strprt

jmp $

; bx: addr of string
stupid_strprt:
mov ah, 0x0e
strprt_loop:
  mov al, [bx]
  cmp al, 0
  je strprt_endloop
  int 0x10
  sub bx,2
  jmp strprt_loop
strprt_endloop:
  push ax
  mov ah, 0x0e
  mov al, 'Z'
  int 0x10
  pop ax
  ret

times 510-($-$$) db 0
db 0x55, 0xaa
