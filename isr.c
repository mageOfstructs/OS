#include "printf.h"
#include "pic.h"
#include <stdint.h>

__attribute__((noreturn)) void exception_handler(void);
__attribute__((noreturn)) void exception_handler_errcode(uint32_t errcode);

typedef struct int_frame {
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
} int_frame_t;

void exception_handler(int_frame_t f) {
  printf("EXCEPTION!\n");
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}

void exception_handler_errcode(uint32_t errcode, int_frame_t f) {
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  printf("ERROR CODE: %d!\n", errcode);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}

void keyboard_test() {
  printf("a\n");
  PIC_sendEOI(1);
}

void int_timer() {
  printf("b\n");
}
