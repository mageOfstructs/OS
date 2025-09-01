#ifndef EXT2_H
#define EXT2_H

#include "../ata.h"
#include "../binops.h"
#include "../printf.h"
#include "../math.h"
#include "../utils.h"
#include "../malloc.h"
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
} superblock_t;

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
  uint16_t gui;
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
  uint32_t hsize; // only avl if version >= 1
  uint32_t frag_ba;
  uint32_t os_specific2[3];
};

#endif // !EXT2_H
