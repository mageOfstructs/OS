#include "ata.h"
#include "printf.h"
#include <stdint.h>

int identify(uint8_t buf[256]) {
  printf("identify start\n");
  outb(IO_BASE + 6, 0xA0);
  for (int i = 2; i < 6; i++) {
    outb(IO_BASE + i, 0);
  }
  io_wait();
  printf("identify 2\n");
  outb(IO_COM, COM_IDENTIFY);
  printf("identify 3\n");
  if (inb(IO_STATUS)) {
    while (inb(IO_STATUS) >= 128)
      ;
    if (inb(0x1F4) || inb(0x1F5))
      return IDENTIFY_NOT_ATA;

    uint8_t byte;
    for (;;) {
      byte = inb(IO_STATUS);
      if (get_bit(&byte, 3) || get_bit(&byte, 0))
        break;
    }
    if (get_bit(&byte, 0))
      return IDENTIFY_ATA_ERR;
    printf("identify: read data start\n");
    for (int i = 0; i < 256; i++) {
      buf[i] = inb(IO_DATA);
    }

    return IDENTIFY_ATA;
  }
  return IDENTIFY_NO_DISK;
}
