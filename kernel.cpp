#include <stdarg.h>

#define uint unsigned long
#define u16 unsigned short
#define u32 unsigned long

// might change in the future (64bit anyone?)
#define ptr_t unsigned long

#define VMEM_START 0xb8000
#define VMEM_COL_WHITE (15 << 4)

void put_char(char c, u16 *off) {
    *off = ((u16)c) | VMEM_COL_WHITE;
}

uint write_str(char *str, u16 *off) {
  uint i = 0;
  while (str[i]) {
    put_char(c, off+i);
  return i;
}

uint write_int(u32 num, uint base, u16 *off) {
  uint ret = 0;
  if (num < 0) {
    put_char('-', off);
    num = -num;
    ret++;
  }
  do {
    uint mod = num % base;
    if (mod < 10)
      put_char('0' + mod, off+ret);
    else
      put_char('a' + mod - 10, off+ret);

    num /= base;
    ret++;
  } while (num > 0);
  return ret;
}

uint write_int10(u32 num, u16 *off) {
  return write_int(num, 10, off);
}

uint write_ptr(ptr_t ptr, u16 *off) {
  put_char('0', off);
  put_char('x', off+1);
  return 2 + write_int(ptr, 16, off+2);
}

uint write_float(double d, u16 *off) {
  uint ret = write_int10((u32) d, off);
  put_char('.', off+ret);
  u32 decimal_part = 0;
  do {
    d *= 10;
    decimal_part = (u32)d;
  } while ((u32)d > 0)
  return ret + 1 + write_int10(decimal_part, off+ret+1);
}

char handle_stage1(u16 **cursor, va_list args, uint *args_i, char format_char) {
  switch (format_char) {
  case 's':
    *cursor += write_str(va_arg(*args_i, char*), *cursor);
    break;
  case 'd':
    *cursor += write_int10(va_arg(*args_i, int), *cursor);
    break;
  case 'p':
    *cursor += write_ptr(va_arg(*args_i, ptr_t), *cursor);
    break;
  case 'f':
    *cursor += write_float(va_arg(*args_i, double), *cursor);
    break;
  default:
    return 0; // ignore all modifiers we don't know
  }
  (*args_i)++;
  return -1; // reset stage; in the future we might have more stages (e.g. padding, floats, etc.)
}

int printf(char* format, ...) {
  va_list args; 
  uint n = 0, i = 0;
  while (format[i]) {
    if (format[i] == '%' && format[i+1] != '%') n += 1;
    i++;
  }
  va_start(args, n);

  unsigned short *cursor = (unsigned short*) VMEM_START;
  
  i = 0;
  char stage = 0;
  uint args_i = 0;
  while (format[i]) {
    if (stage == 1) {
      stage += handle_stage1(cursor, args, &args_i, format[i]);
    }
    switch (format[i]) {
    case '%':
      stage++;
      break;
    default:
      put_char(format[i], cursor);
      cursor++;
    }
    i++;
  }
}

extern "C" void main() {
  *(char *)0xb8000 = 'Q';
  return;
}
