[org 0x7c00]

buf:
  times 10 db 0

; stupid stack
mov bp, 0x8000
mov sp, bp
mov bx, buf
mov cl, 0

loop:
  mov ah, 0
  int 0x16
  
  cmp al, 0x0A ; newline
  je endloop
  
  cmp al, 0x0D ; other newline
  je endloop

  mov [bx], al
  inc bx
  inc cl

  cmp cl, 9
  je endloop
  
  ; push ax
  ; mov ah, 0x0e
  ; int 0x10
  ; pop ax
  
  jmp loop
endloop:
  push 0
  
  ; push ax
  ; mov ah, 0x0e
  ; mov al, 'Z'
  ; int 0x10
  ; pop ax

  mov bx, buf
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
  inc bx
  jmp strprt_loop
strprt_endloop:
  ret

times 510-($-$$) db 0
db 0x55, 0xaa
