#ifndef PRINTF_H

#define PRINTF_H

#define uint unsigned long
#define u16 unsigned short
#define u32 unsigned long

// might change in the future (64bit anyone?)
#define ptr_t unsigned long

#define VMEM_START ((u16 *)0xb8000)
#define VMEM_COL_WHITE (15 << 8)

int printf(const char *format, ...);
uint write_str(const char *str, volatile u16 *off);
void put_char(char c, volatile u16 *off);

#endif // !PRINTF_H
