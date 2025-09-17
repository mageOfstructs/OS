#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#define uint unsigned long

extern void put_char(char c);

uint write_str(const char *str) {
  uint i = 0;
  while (str[i]) {
    put_char(str[i]);
    i++;
  }
  return i;
}

void display_str(const char *str, uint32_t len) {
  for (int i = 0; i < len; i++) {
    put_char(str[i]);
  }
}

uint write_int(long num, uint base) {
  uint ret = 0;
  if (num < 0) {
    put_char('-');
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
    ret += write_str(buf + buf_i + 1);
  else
    ret += write_str("inf");
  return ret;
}

uint write_uint(long num, uint32_t base) {
  if (num < 0)
    num = num * (-1);
  return write_int(num, base);
}

uint write_int10(long num) { return write_int(num, 10); }

uint write_ptr(size_t ptr) {
  put_char('0');
  put_char('x');
  return 2 + write_uint(ptr, 16);
}

uint write_float(double d) {
  uint ret = write_int10((long)d);
  put_char('.');
  uint32_t decimal_part = 0;
  do {
    d *= 10;
    decimal_part = (uint32_t)d;
  } while ((uint32_t)d > 0);
  return ret + 1 + write_int10(decimal_part);
}

char handle_stage1(va_list *args, char format_char) {
  switch (format_char) {
  case 's':
    write_str(va_arg(*args, char *));
    break;
  case 'd':
    write_int10(va_arg(*args, int));
    break;
  case 'u':
    write_uint(va_arg(*args, int), 10);
    break;
  case 'p':
    write_ptr(va_arg(*args, size_t));
    break;
  case 'f':
    write_float(va_arg(*args, double));
    break;
  case 'c':
    put_char(va_arg(*args, int));
    break;
  }
  return -1; // reset stage; in the future we might have more stages (e.g.
             // padding, floats, etc.)
}

int vaprintf(const char *format, va_list args) {
  uint i = 0;
  char stage = 0, cur_line = 0;
  ;
  while (format[i]) {
    if (stage == 1) {
      stage += handle_stage1(&args, format[i]);
      goto loopend;
    }
    switch (format[i]) {
    case '%':
      stage++;
      break;
    default:
      put_char(format[i]);
    }
  loopend:
    i++;
  }

  return 0;
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vaprintf(format, args);
  va_end(args);
}
