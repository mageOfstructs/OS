#include "string.h"
#include <stdint.h>

int strcmp(char *s1, char *s2) {
  while (*s1 && *s2) {
    if (*s1 - *s2)
      return *s1 - *s2;
  }
  if (*s1)
    return 1;
  if (*s2)
    return -1;
  return 0;
}

uint32_t strlen(char *s1) {
  uint32_t ret = 0;
  while (*s1)
    ret++;
  return ret;
}
