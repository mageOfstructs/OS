#include "cursor.h"
#include "serial.h"
#include <stddef.h>
#include <stdint.h>

volatile static uint16_t *cursor = VMEM_START;

void put_char(char c) {
  switch (c) {
  case '\n':
    cursor += VGA_WIDTH - (cursor - VMEM_START) % VGA_WIDTH;
    break;
  case '\r':
    cursor -= (cursor - VMEM_START) % VGA_WIDTH;
    break;
  default:
    *cursor = ((u16)c) | VMEM_COL_WHITE;
    cursor++;
  }
  serial_putc(c);

  if (cursor > LAST_CHAR_ON_SCREEN) { // scroll one line up if we hit the bottom
    size_t last_dst = VMEM_START_CONST;
    for (size_t dst = 0xB80a0; dst < (size_t)LAST_CHAR_ON_SCREEN;
         dst += VGA_WIDTH_BYTES) {
      asm("mov ecx, " CONST_TOSTR(VGA_WIDTH_BYTES) "\n\t"
                                                   "mov esi, %0\n\t"
                                                   "mov edi, %1\n\t"
                                                   "rep; movsb" ::"g"(dst),
          "g"(last_dst));
      last_dst = dst;
    }
    for (size_t i = 0; i < VGA_WIDTH_BYTES; i++) {
      *(char *)(last_dst + i) =
          0; // Screw you clangd, this works perfectly!1!11!11
    }
    cursor = VMEM_START + (VGA_LINES - 1) * VGA_WIDTH;
  }
}
