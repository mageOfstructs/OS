#include "fildes.h"

#include "fs/ext2.h"
#include "vio.h"

typedef union fildes_data {
  fildes_data_ext2_t ext2_data;
  vio_data_t vio_data;
} fildes_data_t;

typedef struct fildes {
  uint32_t cursor;
  uint32_t sz;
  uint8_t type;
  uint8_t perms;
  fildes_data_t data;
} fildes_t;

const fildes_t NULL_FD = { .cursor = 0, .sz = 0, .type = NULL_TYPE, .perms = 0 };

int read(fildes_t *fildes, uint32_t n, void *ret) {
  switch (fildes->type) {
  case EXT2_FILE_TYPE:
    return read_ext2(fildes, n, ret);
  case VIRT_STREAM_TYPE:
  default:
    printf("read: Not implemented: %d!\n", fildes->type);
    return -1;
  }
}
int write(fildes_t *fildes, uint32_t n, const void *src) {
  switch (fildes->type) {
  case EXT2_FILE_TYPE:
    return write_vio(fildes, n, src);
  case VIRT_STREAM_TYPE:
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
  case VIRT_STREAM_TYPE:
    close_vio(fildes);
    break;
  }
}

void seek(fildes_t *fildes, int dir, uint32_t n) {
  if (dir == SEEK_SET) fildes->cursor = n;
  else if (dir == SEEK_CUR) fildes->cursor = fildes->cursor + n;
  else if (dir == SEEK_END) fildes->cursor = fildes->sz - 1 - n;
}
