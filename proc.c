#include "proc.h"
#include "fildes.h"
#include "vio.h"

static uint32_t next_pid = 0;

static uint32_t get_next_pid() { return next_pid++; }

static proc_t PLIST[1024];
static proc_t *curproc;

proc_t *new_proc() {
  proc_t ret = {.pid = get_next_pid(), .fds = {NULL_FD}};
  ret.fds[VIRT_STREAM_STDIN] = open_vio(VIRT_STREAM_STDIN, 0);
  ret.fds[VIRT_STREAM_STDOUT] = open_vio(VIRT_STREAM_STDOUT, 0);
  ret.fds[VIRT_STREAM_STDERR] = open_vio(VIRT_STREAM_STDERR, 0);
  PLIST[ret.pid] = ret;
  return &PLIST[ret.pid];
}

void set_cur_proc(proc_t *p) { curproc = p; }
proc_t *myproc() { return curproc; }
