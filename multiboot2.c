#include "multiboot2.h"
#include "math.h"
#include <stdint.h>

typedef struct multiboot_tag multiboot_tag_t;

void dbg_mb2(uint32_t *mb_info) {
  uint32_t total_sz = *mb_info;
  multiboot_tag_t *mb_tag = (multiboot_tag_t *)(mb_info + 2);
  while (mb_tag->type != MULTIBOOT_TAG_TYPE_END) {
    switch (mb_tag->type) {}
    mb_tag = (void *)mb_tag + pad(mb_tag->size, 8);
  }
}
