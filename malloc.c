#include "malloc.h"
#include "printf.h"

alloc_t init_alloc(char *start, uint32_t sz) {
  uint32_t bitmap_sz = sz / 8 + (sz % 8 > 0 ? 1 : 0);
  for (int i = 0; i < sz / 8 + (sz % 8 > 0 ? 1 : 0);
       i++) { // use first few bytes of heap for bitmap
    *start = 0;
  }
  alloc_t ret = {.bitmap = (uint8_t *)start,
                 .heap_start = (uint32_t)start + bitmap_sz,
                 .sz = sz - bitmap_sz,
                 .bitmap_sz = bitmap_sz};
  return ret;
}

void *alloc_ctx(alloc_t *a, uint32_t sz) {
  if (sz >= a->sz)
    return NULL;
  uint32_t size_of_cur_block = 0;
  uint32_t i = 0;
  while (i < a->bitmap_sz * 8 && size_of_cur_block < sz) {
    // printf("%p ", a->bitmap[i / 8]);
    if (!get_bit(a->bitmap, i++))
      size_of_cur_block++;
    else
      size_of_cur_block = 0;
  }
  if (size_of_cur_block == sz) {
    for (uint32_t j = 0; j < size_of_cur_block; j++) {
      set_bit(a->bitmap, i - j);
    }
    // printf(" %d\n", i);
    return (void *)(a->heap_start + i - size_of_cur_block);
  }
  return NULL;
}

int dealloc_ctx(alloc_t *a, void *start, uint32_t sz) {
  uint32_t bitmap_i = (uint32_t)(start - a->heap_start);
  for (uint32_t i = 0; i < sz; i++) {
    if (get_bit(a->bitmap, bitmap_i + i))
      clear_bit(a->bitmap, bitmap_i + i);
    else {
      for (int j = 0; j < i; j++) {
        set_bit(a->bitmap, bitmap_i + j);
      }
      return -1; // tried deallocating non-allocated memory
    }
  }
  return 0;
}
