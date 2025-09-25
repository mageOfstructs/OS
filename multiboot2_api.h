#ifndef MULTIBOOT2_API_H
#define MULTIBOOT2_API_H
#include <stdint.h>
void dbg_mb2(uint32_t *mb_info);
int mb2_get_load_base_addr(uint32_t *mb_info, uint32_t *ret);

#endif // !MULTIBOOT2_API_H
