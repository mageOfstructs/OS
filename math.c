#include "math.h"

inline uint32_t ceild(uint32_t divid, uint32_t divis) {
  return divid / divis + (divid % divis != 0 ? 1 : 0);
}
