#ifndef VIO_H
#define VIO_H

#include "printf.h"
#include "fildes.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct vio_data {
  uint8_t vio_type;
} vio_data_t;

int write_vio(fildes_t *fildes, uint32_t n, const void *src);
void close_vio(fildes_t *fildes);

#endif // !VIO_H
