#ifndef SYSCALL_H
#define SYSCALL_H

#include "proc.h"
#include <stdint.h>

void sys_test();
void sys_write(uint32_t fd, void *buf, uint32_t sz);
void sys_read(uint32_t fd, void *buf, uint32_t sz);
void sys_exec(char *path);

void sys_fork(proc_ctx_t *ctx);
// void sys_fork();

void sys_exit();

#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_EXEC 11
#define SYS_FORK 12
#define SYS_HLT 99
#define SYS_EXIT 100

#endif // !SYSCALL_H
