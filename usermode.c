#include "usermode.h"
#include "fildes.h"
#include "kstack_alloc.h"
#include "log.h"
#include "malloc.h"
#include "math.h"
#include "printf.h"
#include "proc.h"
#include "utils.h"
#include "vm.h"
#include <stdint.h>

static tss_t TSS;

void setup_tss(gdte_t *gdt) {
  uint32_t tss_addr = (uint32_t)&TSS;
  gdt->lbase = (uint16_t)tss_addr;
  gdt->midbase = (uint8_t)(tss_addr >> 16);
  gdt->hbase = (uint8_t)(tss_addr >> 24);

  uint32_t tss_sz = sizeof(TSS);
  gdt->llimit = (uint16_t)tss_sz;
  gdt->hlimit = (uint8_t)(tss_sz >> 16);

  gdt->accessb = 0x89;
  gdt->flags = 0x4;

  TSS.ss0 = 0x10;
  TSS.esp0 = 0x90000;
  TSS.iomap_base = sizeof(TSS);
}

void avoid_kernel_stack_conflicts(proc_ctx_t ctx) {
  if ((ctx.cs & 3) == 3) { // if we came from ring3
    set_next_kernel_stack(get_next_stack());
  }
}

void set_next_kernel_stack(uint32_t stack) { TSS.esp0 = stack; }

void load_usermode_prog(fildes_t *fd) {
  if (fd->type != EXT2_FILE_TYPE) {
    err("Can only use ext2 fds!");
    return;
  }
  uint32_t userprog_sz = fd->data->ext2_data.sz;
  if (!myproc()->ctx.addr_sp) {
    myproc()->ctx.addr_sp =
        kalloc((ceild(userprog_sz, PG_SIZE) + USER_STACK_PAGES) * 4);
  } else if (myproc()->ctx.addr_sp_sz < userprog_sz) {
    myproc()->ctx.addr_sp = krealloc(
        myproc()->ctx.addr_sp,
        (ceild(myproc()->ctx.addr_sp_sz, PG_SIZE) + USER_STACK_PAGES) * 4,
        (ceild(userprog_sz, PG_SIZE) + USER_STACK_PAGES) * 4);
  }
  myproc()->ctx.addr_sp_sz = userprog_sz;
  uint32_t usermode_start = DEF_USERPROG_START,
           usercode_pages = ceild(userprog_sz, PG_SIZE);
  printf("program size: %d\n", userprog_sz);

  KASSERT(vm_map_buf(myproc()->ctx.addr_sp, usercode_pages, false, true) == 0);
  KASSERT(vm_map_buf(myproc()->ctx.addr_sp + usercode_pages * 4,
                     USER_STACK_PAGES, true, true) == 0);
  vm_map_ext(DEF_USERPROG_START, get_proc_pages(&myproc()->ctx), NULL,
             myproc()->ctx.addr_sp, false, true); // perms are ignored

  KASSERT(read(fd, userprog_sz, (void *)DEF_USERPROG_START) == userprog_sz);
  close_ext2(fd);
  printf("User stack: %p", usermode_start + USER_STACK_PAGES * PG_SIZE - 1);

  uint32_t eip = DEF_USERPROG_START,
           esp = usermode_start + USER_STACK_PAGES * PG_SIZE - 1;
  myproc()->ctx.eip = eip;
  myproc()->ctx.regs.esp = esp;
  myproc()->ctx.regs.ebp = esp;
  jump_usermode((void (*)())DEF_USERPROG_START, esp);
}

void load_new_usermode_prog(fildes_t *fd) {
  if (fd->type != EXT2_FILE_TYPE) {
    err("Can only use ext2 fds!");
    return;
  }
  set_cur_proc(new_proc());
  load_usermode_prog(fd);
}
