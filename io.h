#ifndef IO_H

#define IO_H
#include "printf.h"
#include <stdint.h>

void outb(u16 dev, char val);
char inb(u16 dev);
void io_wait(void);

#endif // !IO_H
