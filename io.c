#include "io.h"

inline void outb(u16 dev, char val) {
  asm volatile("out dx, al" : : [dev] "d"(dev), [val] "a"(val));
}

inline char inb(u16 dev) {
  char ret;
  asm volatile("mov dx, %[dev]\n\t"
               "in %0, dx\n\t"
               // "mov %0, al"
               : "=rm"(ret)
               : [dev] "r"(dev));
  return ret;
}

inline void io_wait(void) { outb(0x80, 0); }
