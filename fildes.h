#ifndef FILDES_H
#define FILDES_H

#define VIRT_STREAM_TYPE 0
#define EXT2_FILE_TYPE 1
#define NULL_TYPE 255

#define SEEK_SET -1
#define SEEK_CUR 0
#define SEEK_END 1

#include "fs/ext2.h"
#include <stdint.h>

typedef struct vio_data {
  uint8_t vio_type;
} vio_data_t;

typedef struct fildes_data_ext2 {
  inode_t *inode;
  uint8_t *buf;
  uint32_t buf_sz; // in blocks
  uint32_t bitmap;
} fildes_data_ext2_t;

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

extern const fildes_t NULL_FD;

int read(fildes_t *fildes, uint32_t n, void *ret);
int write(fildes_t *fildes, uint32_t n, const void *src);
void close(fildes_t *fildes);
void seek(fildes_t *fildes, int dir, uint32_t n);

fildes_t open_ext2(char *path, uint8_t perms);
int read_ext2(fildes_t *fildes, uint32_t n, void *ret);
void close_ext2(fildes_t *fildes);
void init_fs();

#endif // !FILDES_H
