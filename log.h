#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "printf.h"

#define log(format, ...)                                                       \
  printf("%s: " format, __func__ __VA_OPT__(, ) __VA_ARGS__);

#define warn(format, ...)                                                      \
  printf("[WARN file: " __FILE__                                               \
         "; line: " CONST_TOSTR(__LINE__) "] %s: " format,                     \
         __func__ __VA_OPT__(, ) __VA_ARGS__);

#define err(format, ...)                                                       \
  printf("\n[ERR file: " __FILE__                                              \
         "; line: " CONST_TOSTR(__LINE__) "] %s: " format "\n",                \
         __func__ __VA_OPT__(, ) __VA_ARGS__);

#endif // !LOG_H
