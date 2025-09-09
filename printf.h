#ifndef PRINTF_H

#define PRINTF_H

#include <stdint.h>

#define uint unsigned long
#define u16 unsigned short
#define u32 unsigned long

// might change in the future (64bit anyone?)
#define ptr_t unsigned long

#define VMEM_START_CONST 0xb8000
#define VMEM_START ((u16 *)VMEM_START_CONST)
#define VMEM_COL_WHITE (15 << 8)

#define VGA_WIDTH 80
#define VGA_WIDTH_BYTES (VGA_WIDTH * 2)
#define VGA_LINES 25
#define LAST_CHAR_ON_SCREEN                                                    \
  (u16 *)((VGA_WIDTH - 1) * VGA_LINES * 2 + VMEM_START_CONST)

#define _CONST_TOSTR(c) #c
#define CONST_TOSTR(c) _CONST_TOSTR(c)

int printf(const char *format, ...);
void display_str(const char *str, uint32_t len);
void put_char(char c, volatile u16 **off);

#endif // !PRINTF_H
