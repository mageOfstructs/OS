#ifndef ATA_H

#define ATA_H

#include "binops.h"
#include "io.h"

#define IO_BASE 0x1F0
#define IO_DATA (IO_BASE + 0)
#define IO_SCOUNT (IO_BASE + 2)
#define IO_SELECT (IO_BASE + 6)
#define IO_COM (IO_BASE + 7)
#define IO_STATUS (IO_BASE + 7)
#define COM_IDENTIFY 0xEC
#define COM_READ_SECTORS 0x20

#define IDENTIFY_NO_DISK -1
#define IDENTIFY_ATA 0
#define IDENTIFY_NOT_ATA 1
#define IDENTIFY_ATA_ERR 2

#define BYTES_PER_SECTOR 512 // TODO: not how you do it

int identify(uint16_t buf[256]);
uint32_t get_lba_cnt(uint16_t identify_data[256]);
bool lba48_support(uint16_t identify_data[256]);
int read_ata(bool read_master, uint32_t addr, uint32_t size, uint16_t *buf);
#endif // !ATA_H
