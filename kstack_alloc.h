// NOTE: This is a temporary solution and not what we should continue
// However we may as well use this until we get a generic Rust-allocator working
#ifndef KSTACK_ALLOC_H
#define KSTACK_ALLOC_H

#define MIN_STACK 0x9001000
#define MAX_STACK (MIN_STACK + 1023 * 0x1000)
#define KSTACK_SIZE 4096

#include <stdint.h>

uint32_t get_next_stack();
void release_stack(uint32_t stack);

#endif // !KSTACK_ALLOC_H
