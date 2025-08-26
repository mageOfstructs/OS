#include "printf.h"
#include <stdint.h>

__attribute__((noreturn)) void exception_handler(void);
void exception_handler() {
  printf("EXCEPTION!");
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}

__attribute__((noreturn)) void exception_handler(void);
void exception_handler_errcode(uint32_t errcode) {
  printf("ERROR CODE: %d!", errcode);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}
