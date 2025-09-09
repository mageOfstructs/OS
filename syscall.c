#include "syscall.h"
#include "fildes.h"
#include "printf.h"
#include "proc.h"

void sys_write(uint32_t fd, void *buf, uint32_t sz) {
  write(&myproc()->fds[fd], sz, buf);
}
void sys_test() { printf("Hello world from the other side!!\n"); }
