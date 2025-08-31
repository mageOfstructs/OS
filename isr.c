#include "pic.h"
#include "printf.h"
#include <stdint.h>

typedef struct int_frame {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} __attribute__((packed)) int_frame_t;

// __attribute__((noreturn)) void exception_handler(uint16_t inum, int_frame_t f);
__attribute__((noreturn)) void
exception_handler_errcode(uint8_t inum, uint32_t errcode, int_frame_t f);

__attribute__((noreturn)) void exception_handler(uint8_t inum, int_frame_t f) {
  printf("%d\n", inum);
  printf("EXCEPTION!\n");
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}

void exception_handler_errcode(uint8_t inum, uint32_t errcode, int_frame_t f) {
  printf("%d\n", inum);
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  printf("ERROR CODE: %p!\n", errcode);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
}

void keyboard_test(void) {
  char key = get_key_pressed();
  if (key) {
    printf("%c", key);
  }
  PIC_sendEOI(1);
}

void timer(void) {
  // printf("t");
  PIC_sendEOI(0);
}
