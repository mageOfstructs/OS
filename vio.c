#include "vio.h"

int write_vio(fildes_t *fildes, uint32_t n, const void *src) {
  if (fildes->data.vio_data.vio_type == STDOUT) {
    display_str(src, n);
    return n;
  }
  return -1;
}

// stub as we cannot close these streams
void close_vio(fildes_t *fildes) {}
