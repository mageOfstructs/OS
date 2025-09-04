#include "ext2.h"
#include "../mem.h"
#include "../string.h"
#include <stdint.h>

static uint16_t sp_buf[1024];
fs_ext2_ctx_t FS_GLOBAL_CTX;

static inline bool is_ext2_fs(superblock_t *sp) { return sp->sig == EXT2_SIG; }

static inline bool is_dir(inode_t *i) {
  return i->typeperm & 0x4000 ? true : false;
}

int is_fs_not_ok(superblock_t *sp) {
  if (!is_ext2_fs(sp))
    return 1;
  if (sp->fs_state == 2)
    return 1;
  if (sp->fs_state == 2 && sp->err_handling_method != 1)
    return 1;
  if (sp->major_version >= 1) {
    printf("%p", sp->required_features);
    if (sp->required_features & FEAT_REQ_COMP)
      return 2;
    if (sp->required_features & FEAT_REQ_NEEDS_JOURNAL_REPLAY)
      return 4;
    if (sp->required_features & FEAT_REQ_JOURNAL_DEV)
      return 5;
  }
  return 0;
}

static inline uint32_t get_number_of_bgs(superblock_t *sp) {
  uint32_t ret = ceild(sp->total_blocks, sp->blocks_per_bg);
  KASSERT(ret == ceild(sp->total_inodes, sp->inodes_per_bg));
  return ret;
}

static inline uint32_t get_block_size(superblock_t *sp) {
  return 1024 << sp->log2_bs;
}

static inline uint32_t block_to_phys_addr(fs_ext2_ctx_t *ctx,
                                          uint32_t block_addr) {
  return block_addr * ctx->block_sz;
}

static inline uint32_t get_inode_sz(superblock_t *sp) {
  if (sp->major_version >= 1)
    return sp->inode_sz;
  return 128;
}

int __read_block(fs_ext2_ctx_t *ctx, uint32_t bg, uint32_t block,
                 uint16_t *ret) {
  uint32_t real_addr =
      bg * ctx->sp->blocks_per_bg * ctx->block_sz + block * ctx->block_sz;
  return read_ata(true, real_addr, ctx->block_sz, ret);
}

int read_block(uint32_t bg, uint32_t block, uint16_t *ret) {
  return __read_block(&FS_GLOBAL_CTX, bg, block, ret);
}

int read_block_addr(uint32_t block, uint16_t *ret) {
  uint32_t bg = block / FS_GLOBAL_CTX.sp->blocks_per_bg;
  uint32_t bg_off = block % FS_GLOBAL_CTX.sp->blocks_per_bg;
  return read_block(bg, bg_off, ret);
}

void __read_inode(fs_ext2_ctx_t *ctx, uint32_t inode_addr, inode_t *ret) {
  uint32_t block_group = (inode_addr - 1) / ctx->sp->inodes_per_bg;
  uint32_t bg_inodet_index = (inode_addr - 1) % ctx->sp->inodes_per_bg;
  uint32_t containing_block = (bg_inodet_index * ctx->inode_sz) / ctx->block_sz;

  uint16_t *inode_buf = kalloc(ctx->block_sz);
  uint32_t inode_start = bg_inodet_index * ctx->inode_sz;
  printf("Inode BG: %d\n", block_group);
  printf("Inode Table Index: %d\n", bg_inodet_index);
  printf("Inode Table: %d\n", ctx->bgdt[block_group].inode_table_addr);
  printf("Inode Table Adjusted: %d\n", ctx->bgdt[block_group].inode_table_addr +
                                           (inode_start / ctx->block_sz));
  printf("Inode Size: %d\n", ctx->inode_sz);
  printf("inode_start: %d", inode_start);
  printf("inode_start adjusted: %d", inode_start % ctx->block_sz);
  KASSERT(__read_block(ctx, block_group,
                       ctx->bgdt[block_group].inode_table_addr +
                           (inode_start / ctx->block_sz),
                       inode_buf) == 0);
  if (inode_addr == 12) {
    for (int i = 0; i < FS_GLOBAL_CTX.block_sz; i++) {
      if (inode_buf[i])
        printf("%d %p ", i, inode_buf[i]);
    }
  }

  memcpy(&inode_buf[(inode_start % ctx->block_sz) / 2], ret, sizeof(inode_t));
  kfree(inode_buf, ctx->block_sz);
}

void read_inode(uint32_t inode_addr, inode_t *ret) {
  __read_inode(&FS_GLOBAL_CTX, inode_addr, ret);
}

void dbg_inode_dir(fs_ext2_ctx_t *ctx, inode_t *i) {
  printf("Inode Type: %p\n", i->typeperm);
  printf("Inode Size: %d\n", i->lsize);
  int cur_ptr = 0;
  uint16_t *dir_entry_buf = kalloc(ctx->block_sz);
  KASSERT(__read_block(ctx, 0, i->dptrs[0], dir_entry_buf) == 0);
  dir_entry_t *dir_entry = (dir_entry_t *)dir_entry_buf;
  printf("Let's see what's inside the root directory:\n");
  while (((void *)dir_entry - (void *)dir_entry_buf) <
         get_block_size(ctx->sp)) {
    uint32_t name_length =
        dir_entry->lname_length |
        (ctx->dir_have_ti ? 0 : (dir_entry->hname_length << 8));
    if (!name_length) // good enough for now
      goto dir_print_loopend;
    printf("%d ", name_length);

    for (int i = 0; i < name_length; i++) {
      printf("%c", dir_entry->name[i]);
    }
    printf(" %d\n", dir_entry->inode);
  dir_print_loopend:
    dir_entry =
        (void *)dir_entry + (8 + name_length) + 4 - (8 + name_length) % 4;
    cur_ptr++;
  }
  kfree(dir_entry_buf, ctx->block_sz);
}

int __get_inode_from_dir(fs_ext2_ctx_t *ctx, inode_t *dir, char *name,
                         uint32_t name_length, uint32_t *ret) {
  if (!is_dir(dir))
    return -1;
  int cur_ptr = 0;
  uint16_t *dir_entry_buf = kalloc(ctx->block_sz);
  KASSERT(dir_entry_buf);
  printf("Let's see what's inside the root directory:\n");

  // TODO: add indirect pointer support if ya want
  for (int dptri = 0; dptri < 12; dptri++) {
    // TODO: ctx version of this
    KASSERT(read_block_addr(dir->dptrs[dptri], dir_entry_buf) == 0);
    dir_entry_t *dir_entry = (dir_entry_t *)dir_entry_buf;
    while (((void *)dir_entry - (void *)dir_entry_buf) < ctx->block_sz) {
      uint32_t cur_dentry_name_length =
          dir_entry->lname_length |
          (ctx->dir_have_ti ? 0 : (dir_entry->hname_length << 8));
      if (!cur_dentry_name_length) // good enough for now
        goto dir_print_loopend;

      printf("\nNode: ");
      display_str(dir_entry->name, cur_dentry_name_length);
      if (name_length == cur_dentry_name_length &&
          !strcmp(name, dir_entry->name)) {
        *ret = dir_entry->inode;
        return 0;
      }
    dir_print_loopend:
      dir_entry = (void *)dir_entry + (8 + cur_dentry_name_length) + 4 -
                  (8 + cur_dentry_name_length) % 4;
      cur_ptr++;
    }
  }

  kfree(dir_entry_buf, ctx->block_sz);
  return 1;
}

int get_inode_from_dir(inode_t *dir, char *name, uint32_t name_length,
                       uint32_t *ret) {
  return __get_inode_from_dir(&FS_GLOBAL_CTX, dir, name, name_length, ret);
}

int __traverse(fs_ext2_ctx_t *ctx, char *path, inode_t *start_dir,
               inode_t *ret) {
  uint32_t i = 0;
  char *cur_name_start = path;
  uint8_t backslash = 0;

  inode_t *cur_inode = start_dir;
  uint32_t cur_inode_num;

  while (path[i]) {
    switch (path[i]) {
    case '\\':
      backslash++;
      break;
    case '/':
      if (backslash) {
        backslash = 0;
      } else {
        path[i] = '\0';
        i++;
        if (__get_inode_from_dir(ctx, cur_inode, cur_name_start,
                                 i - (uint32_t)cur_name_start + (uint32_t)path -
                                     1,
                                 &cur_inode_num)) {
          return 1;
        __read_inode(ctx, cur_inode_num, cur_inode);
        cur_name_start = path + i;
      }
    }
    i++;
  }

  *ret = *cur_inode;
  return 0;
}

int traverse(char *path, inode_t *start_dir, inode_t *ret) {
  return __traverse(&FS_GLOBAL_CTX, path, start_dir, ret);
}

int __read_from_inode(fs_ext2_ctx_t *ctx, inode_t i, int block_cnt, void *ret) {
  printf("Inode Type: %p\n", i.typeperm);
  printf("Inode Size: %p\n", i.lsize);
  int cur = 0;
  uint32_t bg, bg_offset;
  while (cur < block_cnt) {
    bg = i.dptrs[cur] / ctx->sp->blocks_per_bg;
    bg_offset = i.dptrs[cur] % ctx->sp->blocks_per_bg;
    printf("reading block %d from bg %d\n", bg_offset, bg);
    int rb_ret = __read_block(ctx, bg, bg_offset, ret);
    if (rb_ret)
      break;
    cur++;
    ret += ctx->block_sz;
  }
  return cur;
}

int read_from_inode(uint32_t inode, int block_cnt, void *ret) {
  inode_t i;
  read_inode(inode, &i);
  return __read_from_inode(&FS_GLOBAL_CTX, i, block_cnt, ret);
}

void init_fs() {
  read_ata(true, 1024, 1024, sp_buf);
  superblock_t *sp = (superblock_t *)sp_buf;

  int fs_ok_ret = is_fs_not_ok(sp);
  if (fs_ok_ret) {
    printf("FILESYSTEM NOT OK :c %d\n", fs_ok_ret);
    return;
  }

  FS_GLOBAL_CTX.sp = sp;
  FS_GLOBAL_CTX.inode_sz = get_inode_sz(sp);
  FS_GLOBAL_CTX.block_sz = get_block_size(sp);
  FS_GLOBAL_CTX.dir_have_ti =
      sp->required_features & FEAT_REQ_DIR_HAS_TYPE ? 1 : 0;
  printf("Detected inode size as %d", FS_GLOBAL_CTX.inode_sz);

  printf("Number of blocks: %d\n", sp->total_blocks);
  printf("Number of bytes per block: %d\n", get_block_size(sp));
  printf("Number of inodes: %d\n", sp->total_inodes);
  printf("Number of inodes per bg: %d\n", sp->inodes_per_bg);
  printf("Superblock lives at block: %d\n", sp->superblock_block_num);
  printf("Number of block groups: %d\n", get_number_of_bgs(sp));
  printf("Number of blocks per bgs: %d\n", sp->blocks_per_bg);
  printf("Version: %d.%d\n", sp->major_version, sp->min_version);

  uint32_t bgdt_size = sizeof(bg_desc_t) * get_number_of_bgs(sp);
  bg_desc_t *bgdt = kalloc(bgdt_size);
  KASSERT(bgdt);
  read_ata(true, 2048, bgdt_size, (uint16_t *)bgdt);
  FS_GLOBAL_CTX.bgdt = bgdt;
  printf("%d free inodes in block group 0\n",
         bgdt[0].unallocated_inodes_in_group);
  printf("%d directories in block group 0\n", bgdt[0].dirs_in_group);
  printf("inode table lives at %p\n", bgdt[0].inode_table_addr);

  // uint16_t *inode_buf = kalloc(get_block_size(sp));
  // KASSERT(inode_buf);
  // printf("root inode block addr: %d\n", bgdt[0].inode_table_addr + 1);
  // read_block(sp, 0, bgdt[0].inode_table_addr + 1, inode_buf);
  // for (int i = inode_sz*1; i < sizeof(inode_t) + 0x80; i++)
  //   printf("%p ", inode_buf[i]);

  read_inode(2, FS_GLOBAL_CTX.root); // 2 is the inode of the root directory

  dbg_inode_dir(&FS_GLOBAL_CTX, FS_GLOBAL_CTX.root);
  uint32_t hello_inum;
  printf("awa");
  inode_t hello_node;
  KASSERT(traverse("hello/", FS_GLOBAL_CTX.root, &hello_node) == 0);
  // get_inode_from_dir(FS_GLOBAL_CTX.root, "hello", 5, &hello_inum);
  printf("Hello inode: %d\n", hello_inum);
  void *buf = kalloc(FS_GLOBAL_CTX.block_sz);
  memset(buf, 0, FS_GLOBAL_CTX.block_sz);
  printf("read %d blocks\n",
         __read_from_inode(&FS_GLOBAL_CTX, hello_node, 1, buf));
  printf("%s\n", (char *)buf);
  // read_from_inode(12, 1, buf);
  // printf("%s", (char *)buf);
  kfree(buf, FS_GLOBAL_CTX.block_sz);
}
