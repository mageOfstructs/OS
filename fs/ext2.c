#include "ext2.h"

uint16_t sp_buf[1024];
uint16_t inode_sz;
bool dir_have_ti = false;

static inline bool is_ext2_fs(superblock_t *sp) {
  return sp->sig == EXT2_SIG;
}

int is_fs_not_ok(superblock_t *sp) {
  if (!is_ext2_fs(sp)) return 1;
  if (sp->fs_state == 2) return 1;
  if (sp->fs_state == 2 && sp->err_handling_method != 1) return 1;
  if (sp->major_version >= 1) {
    printf("%p", sp->required_features);
    if (sp->required_features & FEAT_REQ_COMP) return 2;
    if (sp->required_features & FEAT_REQ_NEEDS_JOURNAL_REPLAY) return 4;
    if (sp->required_features & FEAT_REQ_JOURNAL_DEV) return 5;
  }
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

inline uint32_t block_to_phys_addr(superblock_t *sp, uint32_t block_addr) {
  return block_addr * get_block_size(sp);
}

uint32_t get_inode_sz(superblock_t *sp) {
  if (sp->major_version >= 1) return sp->inode_sz;
  return 128;
}

int read_block(superblock_t *sp, uint32_t bg, uint32_t block, uint16_t *ret) {
  uint32_t real_addr = bg * sp->blocks_per_bg * get_block_size(sp) + block * get_block_size(sp);
  return read(true, real_addr, get_block_size(sp), ret);
}

void read_inode(superblock_t *sp, bg_desc_t bgdt[], uint32_t inode_addr, inode_t *ret) {
  uint32_t block_group = (inode_addr - 1) / sp->inodes_per_bg;
  uint32_t bg_inodet_index = (inode_addr - 1) % sp->inodes_per_bg;
  uint32_t containing_block = (bg_inodet_index * inode_sz) / get_block_size(sp);

  uint16_t *inode_buf = kalloc(get_block_size(sp));
  uint32_t inode_start = bg_inodet_index * inode_sz;
  KASSERT(read_block(sp, block_group, bgdt[block_group].inode_table_addr,
                     inode_buf) == 0);

  memcpy(&inode_buf[inode_start / 2], ret, sizeof(inode_t));

  kfree(inode_buf, get_block_size(sp));
}

void dbg_inode(superblock_t *sp, inode_t *i) {
  printf("Inode Type: %p\n", i->typeperm);
  printf("Inode Size: %d\n", i->lsize);
  int cur_ptr = 0;
  uint16_t *dir_entry_buf = kalloc(get_block_size(sp));
  KASSERT(read_block(sp, 0, i->dptrs[0], dir_entry_buf) == 0);
  dir_entry_t *dir_entry = (dir_entry_t*)dir_entry_buf;
  printf("Let's see what's inside the root directory:\n");
  while (cur_ptr < 2) {
    uint32_t name_length = dir_entry->lname_length | (dir_have_ti ? 0 : (dir_entry->hname_length << 8));
    // if (!name_length)
    printf("%d ", name_length);

    for (int i = 0; i < name_length; i++) {
      printf("%c", dir_entry->name[i]);
    }
    dir_entry += (8 + name_length) + 4 - (8 + name_length) % 4;
    cur_ptr++;
    printf("\n");
  }
  kfree(dir_entry_buf, get_block_size(sp));
}


void init_fs() {
  // superblock_t sp;
  read(true, 1024, 1024, sp_buf);
  superblock_t *sp = (superblock_t*)sp_buf;

  int fs_ok_ret = is_fs_not_ok(sp);
  if (fs_ok_ret) {
    printf("FILESYSTEM NOT OK :c %d\n", fs_ok_ret);
    return;
  }

  inode_sz = get_inode_sz(sp);
  dir_have_ti = sp->required_features & FEAT_REQ_DIR_HAS_TYPE ? 1 : 0;

  printf("Number of blocks: %d\n", sp->total_blocks);
  printf("Number of bytes per block: %d\n", get_block_size(sp));
  printf("Number of inodes: %d\n", sp->total_inodes);
  printf("Number of inodes per bg: %d\n", sp->inodes_per_bg);
  printf("Superblock lives at block: %d\n", sp->superblock_block_num);
  printf("Number of block groups: %d\n", get_number_of_bgs(sp));
  printf("Number of blocks per bgs: %d\n", sp->blocks_per_bg);
  printf("Version: %d.%d\n", sp->major_version, sp->min_version);

  uint32_t bgdt_size = sizeof(bg_desc_t)*get_number_of_bgs(sp);
  bg_desc_t *bgdt = kalloc(bgdt_size);
  KASSERT(bgdt);
  read(true, 2048, bgdt_size, bgdt);
  printf("%d free inodes in block group 0\n", bgdt[0].unallocated_inodes_in_group);
  printf("%d directories in block group 0\n", bgdt[0].dirs_in_group);
  printf("inode table lives at %p\n", bgdt[0].inode_table_addr);

  // uint16_t *inode_buf = kalloc(get_block_size(sp));
  // KASSERT(inode_buf);
  // printf("root inode block addr: %d\n", bgdt[0].inode_table_addr + 1);
  // read_block(sp, 0, bgdt[0].inode_table_addr + 1, inode_buf);
  // for (int i = inode_sz*1; i < sizeof(inode_t) + 0x80; i++)
  //   printf("%p ", inode_buf[i]);

  inode_t tmp;
  read_inode(sp, bgdt, 2, &tmp); // 2 is the inode of the root directory
  dbg_inode(sp, &tmp);
}
