#ifndef BINOPS_H

#define BINOPS_H

#include <stdbool.h>
#include <stdint.h>
bool get_bit(uint8_t *bits, uint32_t i);
void set_bit(uint8_t *bits, uint32_t i);
void clear_bit(uint8_t *bits, uint32_t i);

#endif // !BINOPS_H
