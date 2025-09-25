#include "multiboot2.h"
#include "math.h"
#include "log.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct multiboot_tag multiboot_tag_t;
typedef struct multiboot_tag_elf_sections multiboot_tag_elf_sections_t;

static inline multiboot_tag_t *__next_tag(multiboot_tag_t *mb_tag) {
  return (void *)mb_tag + pad(mb_tag->size, 8);
}
static inline bool __tags_not_done(uint32_t total_sz, uint32_t *info_start,
                                   multiboot_tag_t *cur_tag) {
  return cur_tag->type != MULTIBOOT_TAG_TYPE_END &&
         total_sz > ((uint32_t *)cur_tag - info_start);
}
void dbg_mb2(uint32_t *mb_info) {
  uint32_t total_sz = *mb_info;
  multiboot_tag_t *mb_tag = (multiboot_tag_t *)(mb_info + 2);
  while (__tags_not_done(total_sz, mb_info, mb_tag)) {
    log("Enountered type: %d\n", mb_tag->type);
    switch (mb_tag->type) {
    case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:;
      multiboot_tag_elf_sections_t *mb_elf =
          (multiboot_tag_elf_sections_t *)mb_tag;
      log("Found ELF Sections!\n");
      log("Number of entries: %d\n", mb_elf->num);
      break;
    case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
      log("kernel loaded at %p!\n",
          ((struct multiboot_tag_load_base_addr *)mb_tag)->load_base_addr);
      break;
    }
    mb_tag = __next_tag(mb_tag);
  }
}

int mb2_get_load_base_addr(uint32_t *mb_info, uint32_t *ret) {
  uint32_t total_sz = *mb_info;
  multiboot_tag_t *mb_tag = (multiboot_tag_t *)(mb_info + 2);
  while (__tags_not_done(total_sz, mb_info, mb_tag)) {
    switch (mb_tag->type) {
    case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
      *ret = ((struct multiboot_tag_load_base_addr *)mb_tag)->load_base_addr;
      return 0;
    }
    mb_tag = __next_tag(mb_tag);
  }
  return -1;
}
