global prtint
extern write_str

section .text

[bits 32]
strprt:
  ; mov edx, [esp+4] ; get the pointer we received as parameter
  ; mov ecx, 0xb8000
  ; mov ecx, [esp+8]
  mov ah, 0x0f ; set color to white
strprt_loop:
  mov al, [edx]
  
  cmp al, 0
  je strprt_loopend
  
  mov [ecx], ax

  add edx, 2 ; stupid 16-bit stack alignment...
  add ecx, 2
  
  jmp strprt_loop
strprt_loopend:
  ret

[bits 32]
prtint:
  mov eax, [esp+4]
  mov edx, [esp+8] ; base
  mov ecx, [esp+12] ; address in video memory
  mov bh, 0
  push 0
prtint_loop:
  div edx
  mov bl, ah ; move out of ah, since that is needed in the next iteration
  or bl, 48 ; turn modulo into printable character

  push bx
 
  ; nothing to divide anymore
  cmp al, 0
  je prtint_endloop

  mov ah, 0
  jmp prtint_loop
prtint_endloop:
  mov edx, esp
  call strprt
  ret
