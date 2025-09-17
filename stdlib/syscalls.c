#include "syscalls.h"

void write(uint32_t fd, const void *p, uint32_t sz) {
  asm volatile("push %2\n\t"
               "push %1\n\t"
               "push %0 \n\t"
               "mov eax, 1\n\t"
               "int 0x80\n\t" ::"r"(1),
               "m"(p), "r"(sz)
               : "eax", "esp");
}

void read(uint32_t fd, void *buf, uint32_t sz) {
  asm volatile("push %2\n\t"
               "push %1\n\t"
               "push %0 \n\t"
               "mov eax, 2\n\t"
               "int 0x80" ::"r"(0),
               "m"(buf), "r"(sz)
               : "eax", "esp");
}

__attribute__((noreturn)) void exec(char *path) {
  asm volatile("push %0\n\t"
               "mov eax, 11\n\t"
               "int 0x80" ::"m"(path)
               : "eax", "esp");
  for (;;)
    ;
}

inline void fork() {
  asm volatile("mov eax, 12\n\t"
               "int 0x80" ::
                   : "eax");
}

// TODO: exit code
__attribute__((noreturn)) void exit(int exitcode) {
  asm("mov eax, 100\n\t"
      "int 0x80" ::
          : "eax");
  for (;;)
    ;
}
