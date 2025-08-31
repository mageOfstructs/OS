#include "printf.h"
#include <stdint.h>

void outb(uint16_t dev, char val);
uint8_t inb(uint16_t dev);
void io_wait(void);
