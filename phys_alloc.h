#ifndef PHYS_ALLOC_H

#define PHYS_ALLOC_H

#include <stddef.h>
#include <stdint.h>

#define MAX_SMALL_PG_ALLOC_SZ 4

/**
 * struct representing a free (i.e. unmapped) block in physical memory
 **/
typedef struct free_block {
  void *start;
  size_t sz;
} free_block_t;

void *phys_alloc(uint16_t n);
void phys_dealloc(void *allocation);
void init_physalloc(uint32_t heap_start, uint32_t heap_end);

#endif /* ifndef PHYS_ALLOC_H */
