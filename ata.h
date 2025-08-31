#ifndef ATA_H

#define ATA_H

#include "binops.h"
#include "io.h"

#define IO_BASE 0x1F0
#define IO_DATA (IO_BASE + 0)
#define IO_COM (IO_BASE + 7)
#define IO_STATUS (IO_BASE + 7)
#define COM_IDENTIFY 0xEC

#define IDENTIFY_NO_DISK -1
#define IDENTIFY_ATA 0
#define IDENTIFY_NOT_ATA 1
#define IDENTIFY_ATA_ERR 2

int identify(uint8_t buf[256]);
#endif // !ATA_H
