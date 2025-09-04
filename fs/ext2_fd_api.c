#include "../fildes.h"
#include "ext2.h"

extern fs_ext2_ctx_t FS_GLOBAL_CTX;

fildes_t open_ext2(char *path, uint8_t perms) {
  inode_t *i = kalloc(sizeof(inode_t));
  if (!i)
    return NULL_FD;
  if (traverse(path, FS_GLOBAL_CTX.root, i))
    return NULL_FD;
  fildes_t ret = {
      .cursor = 0,
      .sz = i->lsize,
      .type = EXT2_FILE_TYPE,
      .perms = perms,
      .data = {
          .ext2_data = {.inode = i, .buf = NULL, .buf_sz = 0, .bitmap = 0}}};
  return ret;
}

int read_ext2(fildes_t *fildes, uint32_t n, void *ret) {
  fildes_data_ext2_t *data = &fildes->data.ext2_data;
  uint32_t needed_bufsz = ceild(fildes->cursor + n, FS_GLOBAL_CTX.block_sz);
  if (!data->buf) {
    data->buf_sz = needed_bufsz;
    data->buf = kalloc(needed_bufsz * FS_GLOBAL_CTX.block_sz);
  } else if (data->buf_sz < ceild(n, FS_GLOBAL_CTX.block_sz)) {
    data->buf =
        krealloc(data->buf, data->buf_sz,
                 ceild(n, FS_GLOBAL_CTX.block_sz) * FS_GLOBAL_CTX.block_sz);
    data->buf_sz = ceild(n, FS_GLOBAL_CTX.block_sz);
  }

  uint32_t cur_cursor = fildes->cursor;
  uint32_t blocki;
  while (cur_cursor < (fildes->cursor + n + FS_GLOBAL_CTX.block_sz)) {
    blocki = cur_cursor / FS_GLOBAL_CTX.block_sz;
    if (!get_bit((uint8_t *)&data->bitmap, blocki)) {
      set_bit((uint8_t *)&data->bitmap, blocki);
      read_block_addr(data->inode->dptrs[blocki],
                      (uint16_t *)&data->buf[blocki * FS_GLOBAL_CTX.block_sz]);
    }
    cur_cursor += FS_GLOBAL_CTX.block_sz;
  }

  int memcpy_ret = memcpy(&data->buf[fildes->cursor / FS_GLOBAL_CTX.block_sz] +
                              fildes->cursor % FS_GLOBAL_CTX.block_sz,
                          ret, n);
  if (memcpy_ret != -1)
    fildes->cursor += n;
  return memcpy_ret;
}

void close_ext2(fildes_t *fildes) {
  if (fildes->data.ext2_data.buf_sz)
    kfree(fildes->data.ext2_data.buf,
          fildes->data.ext2_data.buf_sz * FS_GLOBAL_CTX.block_sz);
  kfree(fildes->data.ext2_data.inode, sizeof(inode_t));
}
