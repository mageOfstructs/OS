#include "binops.h"

inline bool get_bit(uint8_t *bits, uint32_t i) {
  return (bits[i / 8] >> (i % 8)) & 1;
}
inline void set_bit(uint8_t *bits, uint32_t i) { bits[i / 8] |= 1 << (i % 8); }
inline void clear_bit(uint8_t *bits, uint32_t i) {
  bits[i / 8] &= ~(1 << (i % 8));
}
