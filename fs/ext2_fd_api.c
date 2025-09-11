#include "../fildes.h"
#include "../printf.h"
#include "ext2.h"
#include <stdint.h>

extern fs_ext2_ctx_t FS_GLOBAL_CTX;

fildes_t open_ext2(char *path, uint8_t perms) {
  inode_t *i = kalloc(sizeof(inode_t));
  if (!i)
    return NULL_FD;
  if (traverse(path, FS_GLOBAL_CTX.root, i)) {
    err("traverse fail\n");
    return NULL_FD;
  }
  fildes_data_t *ext2_data_ptr = kalloc(sizeof(fildes_data_t));
  if (!ext2_data_ptr) {
    err("kalloc fail\n");
    return NULL_FD;
  }
  fildes_data_t ext2_data = {.ext2_data = {.cursor = 0,
                                           .sz = i->lsize,
                                           .inode = i,
                                           .buf = NULL,
                                           .buf_sz = 0,
                                           .bitmap = 0}};
  *ext2_data_ptr = ext2_data;
  fildes_t ret = {EXT2_FILE_TYPE, perms, ext2_data_ptr};
  return ret;
}

int read_ext2(fildes_t *fildes, uint32_t n, void *ret) {
  fildes_data_ext2_t *data = &fildes->data->ext2_data;
  // calculate the total number of blocks we will need
  uint32_t needed_bufsz = ceild(data->cursor + n, FS_GLOBAL_CTX.block_sz);
  printf("read_ext2 start\n");
  if (!data->buf) {
    data->buf_sz = needed_bufsz;
    data->buf = kalloc(needed_bufsz * FS_GLOBAL_CTX.block_sz);
  } else if (data->buf_sz < needed_bufsz) {
    data->buf = krealloc(data->buf, data->buf_sz * FS_GLOBAL_CTX.block_sz,
                         needed_bufsz * FS_GLOBAL_CTX.block_sz);
    data->buf_sz = needed_bufsz;
  }
  KASSERT(data->buf);

  uint32_t cur_cursor = data->cursor;
  uint32_t blocki;
  while (cur_cursor < (data->cursor + n)) {
    blocki = cur_cursor / FS_GLOBAL_CTX.block_sz;
    if (!get_bit((uint8_t *)&data->bitmap, blocki)) {
      set_bit((uint8_t *)&data->bitmap, blocki);
      read_block_addr(data->inode->dptrs[blocki],
                      (uint16_t *)&data->buf[blocki * FS_GLOBAL_CTX.block_sz]);
    }
    cur_cursor += FS_GLOBAL_CTX.block_sz;
  }

  int memcpy_ret = memcpy(&data->buf[data->cursor / FS_GLOBAL_CTX.block_sz] +
                              data->cursor % FS_GLOBAL_CTX.block_sz,
                          ret, n);
  if (memcpy_ret > -1)
    data->cursor += n;
  return memcpy_ret;
}

void close_ext2(fildes_t *fildes) {
  if (fildes->data->ext2_data.buf_sz)
    kfree(fildes->data->ext2_data.buf,
          fildes->data->ext2_data.buf_sz * FS_GLOBAL_CTX.block_sz);
  kfree(fildes->data->ext2_data.inode, sizeof(inode_t));
  kfree(fildes->data, sizeof(fildes_data_t));
}

void seek(fildes_t *fildes, int dir, uint32_t n) {
  if (fildes->type != EXT2_FILE_TYPE)
    return;
  fildes_data_ext2_t *data = &fildes->data->ext2_data;
  if (dir == SEEK_SET)
    data->cursor = n;
  else if (dir == SEEK_CUR)
    data->cursor = data->cursor + n;
  else if (dir == SEEK_END)
    data->cursor = data->sz - 1 - n;
}
