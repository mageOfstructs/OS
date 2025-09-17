#include "syscalls.h"
static char buf[256] = {0};
static int bufi = 0;

void put_char(char c) {
  buf[bufi++] = c;
  if (c == '\n' || bufi > 255) {
    write(1, buf, bufi);
    bufi = 0;
  }
}
