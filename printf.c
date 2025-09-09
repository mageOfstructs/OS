#include "printf.h"
#include "cursor.h"
#include "serial.h"
#include <stdarg.h>

volatile static u16 *last_cursor_pos = VMEM_START;

void put_char(char c, volatile u16 **cursor) {
  switch (c) {
  case '\n':
    *cursor += VGA_WIDTH - (*cursor - VMEM_START) % VGA_WIDTH;
    break;
  case '\r':
    *cursor -= (*cursor - VMEM_START) % VGA_WIDTH;
    break;
  default:
    **cursor = ((u16)c) | VMEM_COL_WHITE;
    (*cursor)++;
  }
  serial_putc(c);
}

uint write_str(const char *str, volatile u16 **off) {
  uint i = 0;
  while (str[i]) {
    put_char(str[i], off);
    i++;
  }
  return i;
}

void display_str(const char *str, uint32_t len) {
  for (int i = 0; i < len; i++) {
    put_char(str[i], &last_cursor_pos);
  }
}

uint write_int(long num, uint base, volatile u16 **off) {
  uint ret = 0;
  if (num < 0) {
    put_char('-', off);
    num = -num;
    ret++;
  }

#define MAX_DIGITS 16
  char buf[MAX_DIGITS] = {0};
  uint buf_i = MAX_DIGITS - 2;
  do {
    unsigned char mod = num % base;
    if (mod < 10)
      buf[buf_i] = (char)('0' + mod);
    else
      buf[buf_i] = (char)('a' + mod - 10);

    num /= base;
    buf_i--;
  } while (num > 0 &&
           buf_i < MAX_DIGITS); // handle the case where buf_i underflows
  if (buf_i < MAX_DIGITS)
    ret += write_str(buf + buf_i + 1, off);
  else
    ret += write_str("inf", off);
  return ret;
}

uint write_uint(long num, u32 base, volatile u16 **off) {
  if (num < 0)
    num = num * (-1);
  return write_int(num, base, off);
}

uint write_int10(long num, volatile u16 **off) {
  return write_int(num, 10, off);
}

uint write_ptr(ptr_t ptr, volatile u16 **off) {
  put_char('0', off);
  put_char('x', off);
  return 2 + write_uint(ptr, 16, off);
}

uint write_float(double d, volatile u16 **off) {
  uint ret = write_int10((long)d, off);
  put_char('.', off);
  u32 decimal_part = 0;
  do {
    d *= 10;
    decimal_part = (u32)d;
  } while ((u32)d > 0);
  return ret + 1 + write_int10(decimal_part, off);
}

char handle_stage1(volatile u16 **cursor, va_list *args, char format_char) {
  switch (format_char) {
  case 's':
    write_str(va_arg(*args, char *), cursor);
    break;
  case 'd':
    write_int10(va_arg(*args, int), cursor);
    break;
  case 'u':
    write_uint(va_arg(*args, int), 10, cursor);
    break;
  case 'p':
    write_ptr(va_arg(*args, ptr_t), cursor);
    break;
  case 'f':
    write_float(va_arg(*args, double), cursor);
    break;
  case 'c':
    put_char(va_arg(*args, int), cursor);
    break;
  }
  return -1; // reset stage; in the future we might have more stages (e.g.
             // padding, floats, etc.)
}

int vaprintf(const char *format, va_list args) {
  if (!last_cursor_pos) { // I have no idea why this is, compilers are stupid
    last_cursor_pos = VMEM_START;
  }

  volatile unsigned short *cursor = last_cursor_pos;

  uint i = 0;
  char stage = 0, cur_line = 0;
  ;
  while (format[i]) {
    if (stage == 1) {
      stage += handle_stage1(&cursor, &args, format[i]);
      goto loopend;
    }
    switch (format[i]) {
    case '%':
      stage++;
      break;
    default:
      put_char(format[i], &cursor);
    }
  loopend:
    i++;
  }
  last_cursor_pos = cursor;
  if (cursor > LAST_CHAR_ON_SCREEN) { // scroll one line up if we hit the bottom
    unsigned int last_dst = VMEM_START_CONST;
    for (unsigned int dst = 0xB80a0; dst < (uint32_t)LAST_CHAR_ON_SCREEN;
         dst += VGA_WIDTH_BYTES) {
      asm("mov ecx, " CONST_TOSTR(VGA_WIDTH_BYTES) "\n\t"
                                                   "mov esi, %0\n\t"
                                                   "mov edi, %1\n\t"
                                                   "rep; movsb" ::"g"(dst),
          "g"(last_dst));
      last_dst = dst;
    }
    for (unsigned int i = 0; i < VGA_WIDTH_BYTES; i++) {
      *(char *)(last_dst + i) =
          0; // Screw you clangd, this works perfectly!1!11!11
    }
    last_cursor_pos = VMEM_START + (VGA_LINES - 1) * VGA_WIDTH;
  }
  return 0;
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vaprintf(format, args);
  va_end(args);
}
