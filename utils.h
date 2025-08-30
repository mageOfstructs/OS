#ifndef UTILS_H
#define UTILS_H

#include "printf.h"

#define KASSERT(cond)                                                          \
  do {                                                                         \
    if (!cond) {                                                               \
      printf("ASSERTION FAILED!\n");                                           \
      printf("Location: %s at line %d\n", __FILE__, __LINE__);                 \
      printf("What went wrong: %s\n", #cond);                                  \
      asm("cli; hlt");                                                         \
    }                                                                          \
  } while (0);

#endif // !UTILS_H
