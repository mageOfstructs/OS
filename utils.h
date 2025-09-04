#ifndef UTILS_H
#define UTILS_H

#include "printf.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MWRAP(macro)                                                           \
  do {                                                                         \
    macro                                                                      \
  } while (0);

#define KASSERT(cond)                                                          \
  MWRAP(if (!(cond)) {                                                         \
    printf("ASSERTION FAILED!\n");                                             \
    printf("Location: %s at line %d\n", __FILE__, __LINE__);                   \
    printf("What went wrong: %s\n", #cond);                                    \
    asm("cli; hlt");                                                           \
  })

#define STRUCT_EQ(type)                                                        \
  bool type##_cmp(const type *s1, const type *s2) {                            \
    const size_t struct_size = sizeof(*s1);                                    \
    for (int i = 0; i < struct_size; i++) {                                    \
      if (((uint8_t *)s1)[i] != ((uint8_t *)s2)[i])                            \
        return false;                                                          \
    }                                                                          \
    return true;                                                               \
  }

#endif // !UTILS_H
