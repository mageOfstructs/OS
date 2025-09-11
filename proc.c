#include "proc.h"
#include "fildes.h"
#include "malloc.h"
#include "utils.h"
#include "vio.h"
#include "log.h"
#include "llist.h"
#include "usermode.h"
#include <stdint.h>

static uint32_t next_pid = 0;

static uint32_t get_next_pid() { return next_pid++; }

LIST_DEF(proc_t, proc_t)

static llist_t_proc_t PLIST = LIST_INIT;

static llist_node_proc_t *curproc;
static uint32_t proc_head = 0, cur_procs = 0;

proc_t *new_proc() {
  proc_t ret = {.pid = get_next_pid(), .fds = {NULL_FD}, .state = WAITING};
  ret.fds[VIRT_STREAM_STDIN] = open_vio(VIRT_STREAM_STDIN, 0);
  ret.fds[VIRT_STREAM_STDOUT] = open_vio(VIRT_STREAM_STDOUT, 0);
  ret.fds[VIRT_STREAM_STDERR] = open_vio(VIRT_STREAM_STDERR, 0);
  llist_node_proc_t *n = kalloc(sizeof(llist_node_proc_t));
  KASSERT(n);
  n->val = ret;
  llist_pushb_proc_t(&PLIST, n);
  return &n->val;
}

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
  log("EAX: %p\n", ctx->regs.eax);
  log("EBX: %p\n", ctx->regs.ebx);
  log("ECX: %p\n", ctx->regs.ecx);
  log("EDX: %p\n", ctx->regs.edx);
  log("ESP: %p\n", ctx->regs.esp);
  log("EIP: %p\n", ctx->eip);
}

void switch_proc(llist_node_proc_t *new_proc, proc_ctx_t *old_ctx) {
  memcpy(old_ctx, &curproc->val.ctx, sizeof(proc_ctx_t));
  curproc->val.state = WAITING;

  curproc = new_proc;
  curproc->val.state = ACTIVE;

  proc_ctx_t new_ctx = new_proc->val.ctx;
  asm("mov eax, %0\n\t"
      "mov ebx, %1\n\t"
      "mov ecx, %2\n\t"
      "mov edx, %3\n\t"
      "mov esi, %4\n\t"
      "mov edi, %5\n\t" ::"r"(new_ctx.regs.eax),
      "r"(new_ctx.regs.ebx), "r"(new_ctx.regs.ecx), "r"(new_ctx.regs.edx),
      "r"(new_ctx.regs.esi), "r"(new_ctx.regs.edi));
  printf("Jumping to %p\n", new_proc->val.ctx.eip);
  dbg_ctx(&new_proc->val.ctx);
  jump_usermode((void *)new_proc->val.ctx.eip, new_proc->val.ctx.regs.esp);
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
  }
}
