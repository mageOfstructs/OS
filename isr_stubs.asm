; [bits 32]
%macro isr_err_stub 1
isr_stub_%+%1:
    push byte %1
    call exception_handler_errcode
    iret 
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push byte %1
    call exception_handler
    iret
%endmacro

%macro isr_wrapper 1
global isr_%+%1
extern %1
isr_%+%1:
  pushad
  call %1
  popad
  iret
%endmacro

extern exception_handler
extern exception_handler_errcode
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep

isr_wrapper ata

global isr_test
extern syscall
isr_test:
    pushad
    mov edx, [esp+12+8*4]
    mov [esp+12], edx ; overwrite esp with the one from the interrupt stack frame
    call syscall
    add esp, 8*4
    iret

global isr_keyboard
extern keyboard_test
isr_keyboard:
  pushad
  call keyboard_test
  popad
  iret

global isr_timer
extern timer
isr_timer:
  pushad
  call timer
  popad
  iret

global isr_ignore
isr_ignore:
  iret

global isr_pgf
extern page_flt
isr_pgf:
  mov edx, cr2
  push edx
  call page_flt
  iret
