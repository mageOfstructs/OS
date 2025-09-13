; switches to ctx
; Parameter: proc_ctx_t ctx
global ctx_switch
ctx_switch:
  ; setup iret frame since we may need to switch rings
  xor eax, eax
  mov ax, [esp+40] ; get the cs
  add ax, 8 ; data selector is the next one in the GDT
	mov ds, ax
	mov es, ax 
	mov fs, ax 
	mov gs, ax ; SS is handled by iret
  push eax

  push dword [esp+16+4]

  push dword [esp+44+8] ; new eflags
  push dword [esp+40+12]
  push dword [esp+36+16] ; new eip

  add esp, 20 ; reset esp so the offsets are still valid
  mov eax, [esp+32]
  mov ecx, [esp+28]
  mov edx, [esp+24]
  mov ebx, [esp+20]
  mov ebp, [esp+12]
  mov esi, [esp+8]
  mov edi, [esp+4]
  sub esp, 20 ; reset the reset of esp so iret has the stackframe it expects

  iret
  
