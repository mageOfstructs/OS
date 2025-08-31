#ifndef MALLOC_H
#define MALLOC_H

#include "binops.h"
#include "vm.h"
#include <stdint.h>

typedef struct alloc {
  uint8_t *bitmap;
  uint32_t heap_start;
  uint32_t sz;
  uint32_t bitmap_sz;
} alloc_t;

alloc_t init_alloc(char *start, uint32_t sz);
void *alloc_ctx(alloc_t *a, uint32_t sz);
int dealloc_ctx(alloc_t *a, void *start, uint32_t sz);

#endif // !MALLOC_H
