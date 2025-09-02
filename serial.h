#ifndef SERIAL_H
#define SERIAL_H

#include "io.h"


#define COM1 0x3F8

int init_serial();
void serial_putc(char c);

#endif // !SERIAL_H
