#include "proc.h"
#include "fildes.h"
#include "malloc.h"
#include "utils.h"
#include "vio.h"
#include "log.h"
#include "llist.h"
#include "usermode.h"
#include <stdint.h>

const proc_ctx_t DEF_USER_CTX = {.eip = DEF_USERPROG_START,
                                 .cs = USER_CODE_SEG,
                                 .eflags = DEF_USER_EFLAGS,
                                 .regs = {0}};

static uint32_t next_pid = 0;

static uint32_t get_next_pid() { return next_pid++; }

LIST_DEF(proc_t, proc_t)

static llist_t_proc_t PLIST = LIST_INIT;

static llist_node_proc_t *curproc;
static uint32_t proc_head = 0, cur_procs = 0;

proc_t *new_proc() {
  proc_t ret = {.pid = get_next_pid(),
                .fds = {NULL_FD},
                .state = WAITING,
                .ctx = DEF_USER_CTX};
  ret.fds[VIRT_STREAM_STDIN] = open_vio(VIRT_STREAM_STDIN, 0);
  ret.fds[VIRT_STREAM_STDOUT] = open_vio(VIRT_STREAM_STDOUT, 0);
  ret.fds[VIRT_STREAM_STDERR] = open_vio(VIRT_STREAM_STDERR, 0);
  llist_node_proc_t *n = kalloc(sizeof(llist_node_proc_t));
  KASSERT(n);
  n->val = ret;
  llist_pushb_proc_t(&PLIST, n);
  return &n->val;
}

[[deprecated("PLEASE NO USE")]]
void set_cur_proc(proc_t *p) {
  // oh god please no, if the scheduler breaks this is the reason
  // If you only just slightly modify llist_node so it inserts some padding we
  // are so dead
  printf("llist_node_proc_t sz: %d\n", sizeof(llist_node_proc_t));
  p->state = ACTIVE;
  curproc =
      (llist_node_proc_t *)((uint32_t)p - 2 * sizeof(llist_node_proc_t *));
  KASSERT(&curproc->val == p);
}
proc_t *myproc() { return &curproc->val; }

void dbg_ctx(proc_ctx_t *ctx) {
  printf("EAX: %p\n", ctx->regs.eax);
  printf("EBX: %p\n", ctx->regs.ebx);
  printf("ECX: %p\n", ctx->regs.ecx);
  printf("EDX: %p\n", ctx->regs.edx);
  printf("EBP: %p\n", ctx->regs.ebp);
  printf("ESP: %p\n", ctx->regs.esp);
  printf("EIP: %p\n", ctx->eip);
  printf("CS: %p\n", ctx->cs);
}

void switch_proc(llist_node_proc_t *new_proc, proc_ctx_t *old_ctx) {
  memcpy(old_ctx, &curproc->val.ctx, sizeof(proc_ctx_t));
  curproc->val.state = WAITING;

  curproc = new_proc;
  curproc->val.state = ACTIVE;

  proc_ctx_t new_ctx = new_proc->val.ctx;
  log("New context:\n");
  dbg_ctx(&new_proc->val.ctx);
  ctx_switch(new_ctx);
}

void schedule(proc_ctx_t *old_ctx) {
  if (PLIST.sz < 2)
    return;
  llist_node_proc_t *new_proc = curproc;
  do {
    new_proc = new_proc->next;
    if (!new_proc)
      new_proc = PLIST.head;
  } while (new_proc->val.state != WAITING && new_proc != curproc);
  if (new_proc->val.state == WAITING) {
    printf("Scheduling!\n");
    switch_proc(new_proc, old_ctx);
  } else {
    printf("Everything exited!");
    for (;;)
      asm("cli; hlt");
  }
}
