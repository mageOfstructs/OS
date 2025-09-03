#ifndef VIO_H
#define VIO_H

#include "fildes.h"
#include "printf.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int write_vio(fildes_t *fildes, uint32_t n, const void *src);
void close_vio(fildes_t *fildes);

#endif // !VIO_H
