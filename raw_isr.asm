%macro isr 1
global raw_%1
raw_%1:
  extern %1
  pushad
  cld
  call %1
  popad
  iret
%endmacro

isr int_keyboard
