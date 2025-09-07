#include "mem.h"
#include "printf.h"

int memcpy(const void *src, void *dst, size_t s) {
  if (src == dst)
    return -1;
  else if (src < dst && (char *)src + s > (char *)dst)
    return -2;
  else if (dst < src && (char *)dst + s > (char *)src)
    return -3;
  asm("mov ecx, %0\n\t"
      "mov esi, %1\n\t"
      "mov edi, %2\n\t"
      "rep; movsb\n\t" ::"gr"(s),
      "m"(src), "m"(dst)
      : "ecx", "esi", "edi");
  return s;
}

void memset(void *p, uint8_t b, size_t s) {
  asm("mov ecx, %0\n\t"
      "mov edi, %1\n\t"
      "mov al, %2\n\t"
      "rep; stosb" ::"r"(s),
      "m"(p), "ri"(b)
      : "ecx", "edi", "al");
}
