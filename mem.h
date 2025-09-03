#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>
int memcpy(const void *src, void *dst, size_t s);
void memset(void *p, uint8_t b, size_t s);

#endif // !MEM_H
