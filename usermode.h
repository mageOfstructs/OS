#ifndef USERMODE_H
#define USERMODE_H

#include "fildes.h"
#include "vm.h"
#include <stdint.h>

#define USER_STACK_PAGES 4
#define DEF_USERPROG_START 0x00A00000
#define DEF_USER_EFLAGS 0x3200 // set IOPL to 3 and set interrupt flag
#define USER_DATA_SEG ((4 * 8) | 3)
#define USER_CODE_SEG ((3 * 8) | 3)

typedef struct gdte {
  uint16_t llimit;
  uint16_t lbase;
  uint8_t midbase;
  uint8_t accessb;
  uint8_t hlimit : 4;
  uint8_t flags : 4;
  uint8_t hbase;
} __attribute__((packed)) gdte_t;

typedef struct tss {
  uint32_t link;
  uint32_t esp0;
  uint32_t ss0;
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldtr;
  uint16_t trap;
  uint16_t iomap_base;
} __attribute__((packed)) tss_t;

void setup_tss(gdte_t *gdt);
void set_next_kernel_stack(uint32_t stack);
extern void jump_usermode(void (*f)(), uint32_t esp);
void load_usermode_prog(fildes_t *fd);
void load_new_usermode_prog(fildes_t *fd);

#endif // !USERMODE_H
