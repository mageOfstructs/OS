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

uint32_t get_number_of_bgs(superblock_t *sp) {
  uint32_t ret = ceild(sp->total_blocks, sp->blocks_per_bg);
  KASSERT(ret == ceild(sp->total_inodes, sp->inodes_per_bg));
  return ret;
}

uint32_t get_block_size(superblock_t *sp) {
  return 1024 << sp->log2_bs;
}

uint32_t block_to_phys_addr(superblock_t *sp, uint32_t block_addr) {
  return block_addr * get_block_size(sp);
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
  printf("Number of block groups: %d\n", get_number_of_bgs(sp));
  printf("Version: %d.%d\n", sp->major_version, sp->min_version);

  uint32_t bgdt_size = sizeof(bg_desc_t)*get_number_of_bgs(sp);
  bg_desc_t *bgdt = kalloc(bgdt_size);
  KASSERT(bgdt);
  read(true, 2048, bgdt_size, bgdt);
  printf("%d free inodes in block group 0\n", bgdt[0].unallocated_inodes_in_group);
  printf("%d directories in block group 0\n", bgdt[0].dirs_in_group);
}
