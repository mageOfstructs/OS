#ifndef EXT2_H
#define EXT2_H

#include "../ata.h"
#include "../binops.h"
#include "../printf.h"
#include "../math.h"
#include "../utils.h"
#include "../malloc.h"
#include "../mem.h"
#include <stdbool.h>

#define EXT2_SIG 0xef53

typedef struct superblock {
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t blocks_reserved_for_su;
  uint32_t total_unalloc_blocks;
  uint32_t total_unalloc_inodes;
  uint32_t superblock_block_num;
  uint32_t log2_bs; // log2(block size) - 10
  uint32_t log2_fs; // log2(fragment size) - 10
  uint32_t blocks_per_bg;
  uint32_t fragments_per_bg;
  uint32_t inodes_per_bg;
  uint32_t last_mount; // UNIX timestamp
  uint32_t last_write; // UNIX timestamp
  uint16_t number_of_mounts_since_last_consistency_check; 
  uint16_t number_of_mounts_before_consistency_check; 
  uint16_t sig; // should be equal to EXT2_SIG
  uint16_t fs_state;
  uint16_t err_handling_method;
  uint16_t min_version;
  uint32_t last_consistency_check; // UNIX timestamp
  uint32_t internal_between_consistency_checks; // UNIX time 
  uint32_t os_id;
  uint32_t major_version;
  uint16_t uid_that_can_use_reserved_blocks;
  uint16_t gid_that_can_use_reserved_blocks;
  // extended fields available after version 1
  uint32_t first_non_reserved_inode; // in versions <= 1 this is 11
  uint16_t inode_sz; // in bytes, in versions <= 1 this is 128
  uint16_t superblock_bg_backup;
  uint32_t opt_features;
  uint32_t required_features;
  uint32_t required_write_features;
  uint32_t fs_id[4];
  uint32_t vol_name[4];
  uint32_t path_last_mounted_to[16];
  uint32_t comp_alg;
  uint8_t blocks_to_preallocate_for_files;
  uint8_t blocks_to_preallocate_for_dirs;
  uint16_t unused;
  uint32_t jounal_id[4];
  uint32_t jounal_inode;
  uint32_t journal_dev;
  uint32_t orphan_inode_list_head;
} __attribute__((aligned(1024))) superblock_t;

typedef struct bg_desc {
  uint32_t block_usage_bitmap_ba;
  uint32_t inode_usage_bitmap_ba;
  uint32_t inode_table_addr;
  uint16_t unallocated_blocks_in_group;
  uint16_t unallocated_inodes_in_group;
  uint16_t dirs_in_group;
  uint16_t unused;
  uint32_t unused2[3];
} bg_desc_t;

typedef struct inode {
  uint16_t typeperm;
  uint16_t uid;
  uint32_t lsize;
  uint32_t last_access; // UNIX Timestamp
  uint32_t creation; // UNIX Timestamp
  uint32_t last_modification; // UNIX Timestamp
  uint32_t deletion_time; // UNIX Timestamp
  uint16_t gid;
  uint16_t hardlnk_cnt; // hardlinks to this inode
  uint32_t sec_cnt; // sectors used by this inode
  uint32_t flags;
  uint32_t os_specific;
  uint32_t dptrs[12];
  uint32_t singly_iptr;
  uint32_t doubly_iptr;
  uint32_t triply_iptr;
  uint32_t gen_num;
  uint32_t acl; // only avl if version >= 1
  uint32_t hsize; // only avl if version >= 1 and feature is set
  uint32_t frag_ba;
  uint32_t os_specific2[3];
} inode_t;

typedef struct dir_entry {
  uint32_t inode;
  uint16_t entry_sz;
  uint8_t lname_length; // least significant 8 bits of name length
  uint8_t hname_length; // most significant 8 bits of name length, or Type Indicator if corresponding feature bit is set
  char name[];
} dir_entry_t;

typedef struct fs_ext2_ctx {
  superblock_t *sp;
  bg_desc_t *bgdt;
  uint16_t inode_sz;
  uint16_t block_sz;
  bool dir_have_ti;
} fs_ext2_ctx_t;

#define FEAT_OPT_PREALLOC_BLOCKS 0x1
#define FEAT_OPT_AFS 0x2
#define FEAT_OPT_JOURNAL 0x4
#define FEAT_OPT_INODE_EXTATTRIBS 0x8
#define FEAT_OPT_RESIZE 0x10
#define FEAT_OPT_DIR_HASHINDEX 0x20

#define FEAT_REQ_COMP 0x1
#define FEAT_REQ_DIR_HAS_TYPE 0x2
#define FEAT_REQ_NEEDS_JOURNAL_REPLAY 0x4
#define FEAT_REQ_JOURNAL_DEV 0x8

#define FEAT_RO_SPARSE 0x1
#define FEAT_RO_SIZE64 0x2
#define FEAT_RO_DIR_BINTREE 0x4

#endif // !EXT2_H
