#include "syscall.h"
#include "fildes.h"
#include "printf.h"
#include "proc.h"

void sys_write(uint32_t fd, void *buf, uint32_t sz) {
  write(&myproc()->fds[fd], sz, buf);
}
void sys_read(uint32_t fd, void *buf, uint32_t sz) {
  printf("Reading %d bytes from fd %d into %p\n", sz, fd, buf);
  read(&myproc()->fds[fd], sz, buf);
}
void sys_test() { printf("Hello world from the other side!!\n"); }
