global IDT_start
extern init_idt

setup_idt:
  call init_idt
  lidt [IDT_descriptor]
  ret

IDT_start:
; initialize idt with all zeroes
times 256 dd 0
times 256 dd 0
IDT_end:

IDT_descriptor:
  dw IDT_end - IDT_start - 1
  dd IDT_start
