#include "syscall.h"
#include "fildes.h"
#include "malloc.h"
#include "printf.h"
#include "fd_perms.h"
#include "proc.h"
#include "usermode.h"
#include "vm.h"
#include <stdint.h>

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
}

// void sys_fork() {
//   // we don't want to get interrupted by the timer while
//   // PLIST is in an invalid state
//   asm volatile("cli"); // maybe make this section smaller?
//   proc_t *np = new_proc();
//
//   np->ctx.addr_sp_sz = myproc()->ctx.addr_sp_sz;
//   uint32_t usercode_pages = ceild(np->ctx.addr_sp_sz, PG_SIZE);
//   KASSERT(np->ctx.addr_sp =
//               kalloc((np->ctx.addr_sp_sz + USER_STACK_PAGES) * 4));
//   // TODO : we currently don't get any writable data segment as
//   // load_usermode_prog is too naive for that
//   KASSERT(vm_map_buf(np->ctx.addr_sp, usercode_pages, false, true) == 0);
//   KASSERT(vm_map_buf(np->ctx.addr_sp + usercode_pages * 4, USER_STACK_PAGES,
//                      true, true) == 0);
//
//   // setup the user stack
//   np->ctx.regs.ebp = DEF_USERPROG_START + usercode_pages * PG_SIZE +
//                      USER_STACK_PAGES * PG_SIZE - 1;
//   np->ctx.regs.esp = np->ctx.regs.ebp;
//   asm volatile("sti");
// }

void sys_fork(proc_ctx_t *ctx) {
  asm volatile("cli"); // maybe make this section smaller?
  proc_t *np = new_proc();

  np->ctx.addr_sp_sz = myproc()->ctx.addr_sp_sz;
  uint32_t usercode_pages = ceild(np->ctx.addr_sp_sz, PG_SIZE);
  KASSERT(np->ctx.addr_sp =
              kalloc((np->ctx.addr_sp_sz + USER_STACK_PAGES) * 4));
  // TODO : we currently don't get any writable data segment as
  // load_usermode_prog is too naive for that
  KASSERT(vm_map_buf(np->ctx.addr_sp, usercode_pages, false, true) == 0);
  KASSERT(vm_map_buf(np->ctx.addr_sp + usercode_pages * 4, USER_STACK_PAGES,
                     true, true) == 0);

  // setup the user stack
  memcpy(ctx, &np->ctx, sizeof(proc_ctx_t) - 8);
  asm volatile("sti");
}

// TODO: exit code
void sys_exit() {
  myproc()->state = DEAD;
  rm_curproc();
  dispatch_curproc();
}
