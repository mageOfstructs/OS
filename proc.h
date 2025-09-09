#ifndef PROC_H
#define PROC_H

#include "fildes.h"
#include <stdint.h>
#define MAX_FD 128

// TODO: this should store the context at some point
typedef struct proc {
  uint32_t pid;
  fildes_t fds[MAX_FD];
} proc_t;

proc_t *new_proc();
void set_cur_proc(proc_t *p);
proc_t *myproc();

#endif // !PROC_H
