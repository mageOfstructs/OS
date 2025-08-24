[org 0x7c00]

; mov bp, 0x8000
; mov sp, bp
; mov bh, 'A'
; push bx
; 
; mov bh, 'B'
; mov ah, 0x0e
; mov al, bh
; int 0x10
; 
; pop bx
; mov ah, 0x0e
; mov al, bh
; int 0x10

mov bx, hello
call strprt
mov bx, furry
call strprt

jmp $

hello:
  db "Hello World", 0
furry:
  db "UWU", 0

; prints a NUL-terminated string
; bx: pointer to str
strprt:
  mov ah, 0x0e
loop:
  mov al, [bx]
  cmp al, 0
  je endloop
  int 0x10
  inc bx
  jmp loop
endloop:
  ret

times 510-($-$$) db 0
db 0x55, 0xaa
