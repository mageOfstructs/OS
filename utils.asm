global strprt

section .text

[bits 32]
strprt:
  mov ecx, 0xb8000
  mov edx, [esp+4] ; get the pointer we received as parameter
  mov ah, 0x0f ; set color to white
strprt_loop:
  mov al, [edx]
  
  cmp al, 0
  je strprt_loopend
  
  mov [ecx], ax

  inc edx
  add ecx, 2
  
  jmp strprt_loop
strprt_loopend:
  ret
