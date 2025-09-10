#ifndef STRING_H
#define STRING_H

#include <stdint.h>

int strcmp(char *s1, char *s2);
int strcmp_len(char *s1, char *s2, uint32_t len);
uint32_t strlen(char *s1);

#endif // !STRING_H
