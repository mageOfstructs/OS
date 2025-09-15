#include "ata.h"
#include "pic.h"
#include "printf.h"
#include "proc.h"
#include "syscall.h"
#include <stdint.h>

typedef struct int_frame {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} __attribute__((packed)) int_frame_t;

__attribute__((noreturn)) void exception_handler(uint8_t inum, int_frame_t f) {
  printf("%d\n", inum);
  printf("EXCEPTION!\n");
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
  for (;;)
    ;
}

__attribute__((noreturn)) void
exception_handler_errcode(uint8_t inum, uint32_t errcode, int_frame_t f) {
  printf("%d\n", inum);
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  printf("ERROR CODE: %p!\n", errcode);
  __asm__ volatile("cli"); // Completely hangs the computer
  for (;;)
    asm("hlt");
}

void page_flt(uint32_t vaddr, uint32_t errcode, int_frame_t f) {
  printf("PAGE FAULT CAUSED BY %p\n", vaddr);
  printf("EIP: %p; CS: %p; EFLAGS: %p\n", f.eip, f.cs, f.eflags);
  printf("ERROR CODE: %p!\n", errcode);
  __asm__ volatile("cli; hlt; jmp $"); // Completely hangs the computer
}

int keybuf_i = 0;
char keybuf[64];
void keyboard_test(void) {
  char key = get_key_pressed();
  if (key) {
    printf("%c", key);
    keybuf[keybuf_i++] = key;
  }
  PIC_sendEOI(1);
}

#define USTACK_PARAM_FN_NAME(type, name)                                       \
  static inline type __ustack_##name(void **ustack) {                          \
    type ret = *((type *)*ustack);                                             \
    *ustack += sizeof(type);                                                   \
    return ret;                                                                \
  }
#define USTACK_PARAM_FN(type) USTACK_PARAM_FN_NAME(type, type)

// USTACK_PARAM_FN(int)
// USTACK_PARAM_FN(uint32_t)
// USTACK_PARAM_FN_NAME(void *, ptr)

void syscall(proc_ctx_t ctx) {
  void *ustack = (void *)ctx.regs.esp;
  uint32_t sysnum = ctx.regs.eax;
  // asm("mov %0, eax" : "=r"(sysnum) :);
  printf("syscall (%d) got stack %p\n", sysnum, ustack);
  switch (sysnum) {
  case SYS_WRITE:
    sys_write(*((uint32_t *)ustack), *((void **)(ustack + 4)),
              *((uint32_t *)(ustack + 8)));
    break;
  case SYS_READ:
    sys_read(*((uint32_t *)ustack), *((void **)(ustack + 4)),
             *((uint32_t *)(ustack + 8)));
    break;
  case SYS_EXEC:
    sys_exec(*((char **)ustack));
    break;
  case SYS_FORK:
    sys_fork(&ctx);
    break;
  case SYS_EXIT:
    sys_exit();
    break;
  case SYS_HLT:
    for (;;)
      asm("hlt");
    break;
  default:
    sys_test();
  }
}

void timer(proc_ctx_t ctx) {
  // printf("t");
  scheduler(&ctx);
  PIC_sendEOI(0);
}

void ata(void) {
  // printf("ATA IRQ\n");
  PIC_sendEOI(14);
}
