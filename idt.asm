global IDT_start
global setup_idt
extern init_idt
extern PIC_remap

section .data
IDT_start:
; initialize idt with all zeroes
times 256 dd 0
times 256 dd 0
IDT_end:

IDT_descriptor:
  dw IDT_end - IDT_start - 1
  dd IDT_start

IDT_desc_ptr: dd IDT_descriptor

section .text
setup_idt:
  call init_idt
  ; mov eax, 0xB8000
  ; mov bl, [IDT_start+0x21*8+5]
  ; mov byte [eax], bl
  lidt [IDT_desc_ptr]

  push 0x28 ; slave pic offset
  push 0x20 ; master pic offset
  call PIC_remap
  times 2 pop eax
  ret
