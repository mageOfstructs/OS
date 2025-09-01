#include "ata.h"
#include "printf.h"
#include "binops.h"
#include <stdint.h>
#include <stdbool.h>

static inline void wait_while_busy() { while (inb(IO_STATUS) > 127);}

static uint8_t poll() {
  uint8_t stat;
  for (int i = 0; i < 4; i++) {
    stat = inb(IO_STATUS);
    if (!get_bit(&stat, 7) && get_bit(&stat, 5)) // DF/ERR could still be set accidentally for the first four reads
      return stat;
  }
  for (;;) {
    stat = inb(IO_STATUS);
    if (get_bit(&stat, 0) | get_bit(&stat, 3) | (!get_bit(&stat, 7) && get_bit(&stat, 5)))
      return stat;
  }
}

static uint8_t delay400ns() {
  for (int i = 0; i < 13; i++) inb(IO_STATUS);
  return inb(IO_STATUS);
}

static uint8_t drv_select(bool select_master) {
  outb(IO_SELECT, select_master ? 0xA0 : 0xB0);
  delay400ns();
}

static void cache_flush() {
  outb(IO_COM, 0xE7);
  wait_while_busy();
}

int identify(uint16_t buf[256]) {
  printf("identify start\n");
  if (inb(IO_STATUS) == 0xff) return IDENTIFY_NO_DISK;
  outb(IO_STATUS, 0); // is supposed to clear previous errors
  drv_select(true);  
  for (int i = 1; i < 6; i++) {
    outb(IO_BASE + i, 0);
  }
  io_wait();

  outb(IO_COM, COM_IDENTIFY);
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
      buf[i] = inw(IO_DATA);
    }

    return IDENTIFY_ATA;
  }
  return IDENTIFY_NO_DISK;
}

// reads sectors using a 28LBA
void read_internal(uint32_t lba, uint8_t sec_cnt, uint16_t *buf) {
  uint32_t bufi = 0;
  outb(IO_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
  outb(IO_SCOUNT, sec_cnt);
  outb(0x1F3, (uint8_t)lba);
  outb(0x1F4, (uint8_t)(lba >> 8));
  outb(0x1F5, (uint8_t)(lba >> 16));
  outb(IO_COM, COM_READ_SECTORS);

  for (int i = 0; i < sec_cnt; i++) {
    poll();
    do {
      buf[bufi++] = inw(IO_DATA);
    } while (bufi % 256 > 0);
    delay400ns();
  }
}

// easy API for reading bytes into buf
// addr is starting address in bytes (must be aligned to BYTES_PER_SECTOR for now)
// size is the size of sectors in bytes
int read(bool read_master, uint32_t addr, uint32_t size, uint16_t *buf) {
  if (!read_master) {
    printf("read: TODO!\n");
    return -1;
  }
  read_internal(addr / BYTES_PER_SECTOR, size / BYTES_PER_SECTOR + (size % BYTES_PER_SECTOR > 0 ? 1 : 0), buf);
  return 0;
}

uint32_t get_lba_cnt(uint16_t identify_data[256]) {
  const int lba_cnt_off = 60;
  return ((uint32_t)identify_data[lba_cnt_off]) | ((uint32_t)identify_data[lba_cnt_off+1] << 16);
}

bool lba48_support(uint16_t identify_data[256]) {
  return (identify_data[83] >> 10) & 1;
}

