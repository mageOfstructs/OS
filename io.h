#include "printf.h"
#include <stdint.h>

void outb(uint16_t dev, char val);
char inb(uint16_t dev);
void io_wait(void);
