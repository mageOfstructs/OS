#include "malloc.h"
#include "printf.h"

void dbg_print_bitmap(alloc_t *a) {
  for (int i = 0; i < a->bitmap_sz; i++) {
    printf("%p ", a->bitmap[i]);
  }
}

alloc_t kalloc_alloc;

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
    if (!get_bit(a->bitmap, i++)) 
      size_of_cur_block++;
    else
      size_of_cur_block = 0;
  }
  if (size_of_cur_block == sz) {
    for (uint32_t j = 1; j <= size_of_cur_block; j++) {
      set_bit(a->bitmap, i - j);
    }
    // printf("malloc: allocated %d bytes at %d\n", sz, i - size_of_cur_block);
    return (void *)(a->heap_start + i - size_of_cur_block);
  }
  return NULL;
}

int dealloc_ctx(alloc_t *a, void *start, uint32_t sz) {
  // printf("malloc: deallocating %d bytes starting from %p\n", sz, start - a->heap_start);
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

void init_kalloc(char *start, uint32_t sz) {
  kalloc_alloc = init_alloc(start, sz);
}

void *kalloc(uint32_t sz) {
  return alloc_ctx(&kalloc_alloc, sz);
}

int kfree(void *start, uint32_t sz) {
  return dealloc_ctx(&kalloc_alloc, start, sz);
}
