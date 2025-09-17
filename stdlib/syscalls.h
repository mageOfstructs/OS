#ifndef STDLIB_SYSCALL_H
#define STDLIB_SYSCALL_H

#include <stdint.h>
void write(uint32_t fd, const void *p, uint32_t sz);
void read(uint32_t fd, void *buf, uint32_t sz);
__attribute__((noreturn)) void exec(char *path);
extern inline void fork();
__attribute__((noreturn)) inline void exit(int exitcode);

#endif // !STDLIB_SYSCALL_H
