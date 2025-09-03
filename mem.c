#include "mem.h"

int memcpy(const void *src, void *dst, size_t s) {
  if (src == dst) return -1;
  else if (src < dst && (char*)src + s > (char*)dst) return -1;
  else if ((char*)dst + s > (char*)src) return -1;
  asm("mov ecx, %0\n\t"
      "mov esi, %1\n\t"
      "mov edi, %2\n\t"
      "rep; movsb\n\t" :: "gr"(s), "m"(src), "m"(dst) : "ecx", "esi", "edi");
  return s;
}
