#ifndef PROC_H
#define PROC_H

#include "fildes.h"
#include <stdint.h>
#define MAX_FD 128

typedef enum proc_state { ACTIVE, WAITING, BLOCKED, DEAD } proc_state_t;

typedef struct regs {
  uint32_t edi;
  uint32_t esi;

  uint32_t ebp;
  uint32_t esp;

  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
} regs_t;

// intended to be directly mapped onto some stack
typedef struct proc_ctx {
  regs_t regs;

  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;

  void *addr_sp;
  uint32_t addr_sp_sz; // NOTE: ONLY takes code segment into account!
} proc_ctx_t;

extern const proc_ctx_t DEF_USER_CTX;

typedef struct proc {
  uint32_t pid;
  proc_ctx_t ctx;
  fildes_t fds[MAX_FD];
  proc_state_t state;
} proc_t;

typedef struct node llist_node_proc_t;

proc_t *new_proc();
void set_cur_proc(proc_t *p);
proc_t *myproc();
void rm_curproc();
void dispatch_curproc();
void scheduler(proc_ctx_t *old_ctx);
void dbg_ctx(proc_ctx_t *ctx);
int update_curproc_ctx(proc_ctx_t *ctx);
uint32_t get_proc_pages(proc_ctx_t *ctx);
llist_node_proc_t *schedule();
void dispatch(llist_node_proc_t *new_proc);

extern __attribute__((noreturn)) void ctx_switch(proc_ctx_t ctx);

#endif // !PROC_H
