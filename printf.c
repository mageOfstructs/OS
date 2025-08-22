#include <stdarg.h>

#define uint unsigned long
#define u16 unsigned short
#define u32 unsigned long

// might change in the future (64bit anyone?)
#define ptr_t unsigned long

#define VMEM_START 0xb8000
#define VMEM_COL_WHITE (15 << 8)

extern void prtint(u32 num, u32 base, u16 *off);

void put_char(char c, u16 *off) { *off = ((u16)c) | VMEM_COL_WHITE; }

uint write_str(char *str, u16 *off) {
  uint i = 0;
  while (str[i]) {
    put_char(str[i], off + i);
    i++;
  }
  return i;
}

uint write_int(u32 num, uint base, u16 *off) {
  uint ret = 0;
  if (num < 0) {
    put_char('-', off);
    num = -num;
    ret++;
  }
  prtint(num, base, off);
  // do {
  //   uint mod = num % base;
  //   if (mod < 10)
  //     put_char('0' + mod, off + ret);
  //   else
  //     put_char('a' + mod - 10, off + ret);
  //
  //   num /= base;
  //   ret++;
  // } while (num > 0);
  return ret;
}

uint write_int10(u32 num, u16 *off) { return write_int(num, 10, off); }

uint write_ptr(ptr_t ptr, u16 *off) {
  put_char('0', off);
  put_char('x', off + 1);
  return 2 + write_int(ptr, 16, off + 2);
}

uint write_float(double d, u16 *off) {
  uint ret = write_int10((u32)d, off);
  put_char('.', off + ret);
  u32 decimal_part = 0;
  do {
    d *= 10;
    decimal_part = (u32)d;
  } while ((u32)d > 0);
  return ret + 1 + write_int10(decimal_part, off + ret + 1);
}

char handle_stage1(u16 **cursor, va_list args, char format_char) {
  switch (format_char) {
  case 's':
    *cursor += write_str(va_arg(args, char *), *cursor);
    break;
  case 'd':
    *cursor += write_int10(va_arg(args, int), *cursor);
    return 0;
    break;
  case 'p':
    *cursor += write_ptr(va_arg(args, ptr_t), *cursor);
    break;
  case 'f':
    *cursor += write_float(va_arg(args, double), *cursor);
    break;
  }
  return -1; // reset stage; in the future we might have more stages (e.g.
             // padding, floats, etc.)
}

#pragma GCC diagnostic ignored "-Wincompatible-library-redeclaration"
int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  unsigned short *cursor = (unsigned short *)VMEM_START;

  uint i = 0;
  char stage = 0;
  while (format[i]) {
    if (stage == 1) {
      stage += handle_stage1(&cursor, args, format[i]);
      goto loopend;
    }
    switch (format[i]) {
    case '%':
      stage++;
      break;
    default:
      put_char(format[i], cursor);
      cursor++;
    }
  loopend:
    i++;
  }
  va_end(args);
  return 0;
}
