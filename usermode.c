#include "usermode.h"
#include "fildes.h"
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

void load_usermode_prog(fildes_t *fd) {
  uint32_t usermode_start = DEF_USERPROG_START,
           usercode_pages = ceild(fd->sz, PG_SIZE);
  printf("program size: %d\n", fd->sz);
  KASSERT(vm_map_ext(usermode_start, usercode_pages, NULL, NULL, false, true) ==
          0);
  usermode_start += usercode_pages * PG_SIZE;
  KASSERT(vm_map_ext(usermode_start, USER_STACK_PAGES, NULL, NULL, true,
                     true) == 0);

  KASSERT(read(fd, fd->sz, (void *)DEF_USERPROG_START) == fd->sz);
  set_cur_proc(new_proc());
  printf("User stack: %p", usermode_start + USER_STACK_PAGES * PG_SIZE - 1);
  jump_usermode((void (*)())DEF_USERPROG_START,
                usermode_start + USER_STACK_PAGES * PG_SIZE - 1);
}
