#include "syscall.h"
#include "fildes.h"
#include "printf.h"
#include "fd_perms.h"
#include "proc.h"
#include "usermode.h"

void sys_write(uint32_t fd, void *buf, uint32_t sz) {
  write(&myproc()->fds[fd], sz, buf);
}
void sys_read(uint32_t fd, void *buf, uint32_t sz) {
  printf("Reading %d bytes from fd %d into %p\n", sz, fd, buf);
  read(&myproc()->fds[fd], sz, buf);
}
void sys_test() { printf("Hello world from the other side!!\n"); }

void sys_exec(char *path) {
  fildes_t new_prog_fd = open_ext2(path, O_RDONLY);
  if (fildes_t_cmp(&new_prog_fd, &NULL_FD)) {
    err("Could not open path %s", path);
    return;
  }
  load_usermode_prog(&new_prog_fd);
  close_ext2(&new_prog_fd); // FIXME: this will never execute
}
