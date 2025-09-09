#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

void sys_test();
void sys_write(uint32_t fd, void *buf, uint32_t sz);
void sys_read(uint32_t fd, void *buf, uint32_t sz);

#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_HLT 99

#endif // !SYSCALL_H
