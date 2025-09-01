#include "ext2.h"

static inline bool is_ext2_fs(superblock_t *sp) {
  return sp->sig == EXT2_SIG;
}

int is_fs_not_ok(superblock_t *sp) {
  if (!is_ext2_fs(sp)) return 1;
  if (sp->fs_state == 2) return 1;
  if (sp->fs_state == 2 && sp->err_handling_method != 1) return 1;
  return 0;
}

uint32_t get_blocks_per_bg(superblock_t *sp) {
  uint32_t ret = ceild(sp->total_blocks, sp->blocks_per_bg);
  KASSERT(ret == ceild(sp->total_inodes, sp->inodes_per_bg));
  return ret;
}

uint32_t get_block_size(superblock_t *sp) {
  return 1024 << sp->log2_bs;
}

uint16_t sp_buf[1024];

void init_fs() {
  // superblock_t sp;
  read(true, 1024, 1024, sp_buf);
  superblock_t *sp = (superblock_t*)sp_buf;

  int fs_ok_ret = is_fs_not_ok(sp);
  if (fs_ok_ret) {
    printf("FILESYSTEM NOT OK :c %d\n", fs_ok_ret);
    return;
  }
  printf("Number of blocks: %d\n", sp->total_blocks);
  printf("Number of bytes per block: %d\n", get_block_size(sp));
  printf("Number of inodes: %d\n", sp->total_inodes);
  printf("Superblock lives at block: %d\n", sp->superblock_block_num);
  printf("Number of block groups: %d\n", get_blocks_per_bg(sp));
}
