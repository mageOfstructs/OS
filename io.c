#include "io.h"

inline void outb(uint16_t dev, char val) {
  asm volatile("out dx, al" : : [dev] "d"(dev), [val] "a"(val));
}

inline uint8_t inb(uint16_t dev) {
  char ret;
  asm volatile("mov dx, %[dev]\n\t"
               "in %0, dx\n\t"
               // "mov %0, al"
               : "=rm"(ret)
               : [dev] "r"(dev));
  return ret;
}

inline uint16_t inw(uint16_t dev) {
  uint16_t ret;
  asm volatile("mov dx, %[dev]\n\t"
               "in %0, dx\n\t"
               // "mov %0, al"
               : "=rm"(ret)
               : [dev] "r"(dev));
  return ret;
}

inline void io_wait(void) { outb(0x80, 0); }
