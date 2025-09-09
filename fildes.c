#include "fildes.h"

#include "vio.h"

const fildes_t NULL_FD = {.type = NULL_TYPE, .perms = 0};

int read(fildes_t *fildes, uint32_t n, void *ret) {
  switch (fildes->type) {
  case EXT2_FILE_TYPE:
    return read_ext2(fildes, n, ret);
  case VIRT_STREAM_STDIN:
    return read_vio(fildes, n, ret);
  case VIRT_STREAM_STDERR:
  case VIRT_STREAM_STDOUT:
  default:
    printf("read: Not implemented: %d!\n", fildes->type);
    return -1;
  }
}
int write(fildes_t *fildes, uint32_t n, const void *src) {
  switch (fildes->type) {
  case VIRT_STREAM_STDERR:
  case VIRT_STREAM_STDOUT:
  case VIRT_STREAM_STDIN:
    return write_vio(fildes, n, src);
  case EXT2_FILE_TYPE:
  default:
    printf("write: Not implemented: %d!\n", fildes->type);
    return -1;
  }
}

void close(fildes_t *fildes) {
  switch (fildes->type) {
  case EXT2_FILE_TYPE:
    close_ext2(fildes);
    break;
  case VIRT_STREAM_STDERR:
  case VIRT_STREAM_STDOUT:
  case VIRT_STREAM_STDIN:
    close_vio(fildes);
    break;
  }
}
