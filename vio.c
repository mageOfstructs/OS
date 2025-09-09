#include "vio.h"
#include "fildes.h"
#include <stdint.h>

fildes_t open_vio(uint32_t stream, uint8_t perms) {
  fildes_t ret = {.cursor = 0,
                  .sz = -1,
                  .type = VIRT_STREAM_TYPE,
                  .perms = perms,
                  .data = {.vio_data = {.vio_type = stream}}};
  return ret;
}

int write_vio(fildes_t *fildes, uint32_t n, const void *src) {
  if (fildes->data.vio_data.vio_type == STDOUT) {
    display_str(src, n);
    return n;
  }
  return -1;
}

// stub as we cannot close these streams
void close_vio(fildes_t *fildes) {}
