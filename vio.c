#include "vio.h"
#include "mem.h"
#include "fildes.h"
#include <stdint.h>

fildes_t open_vio(uint32_t stream, uint8_t perms) {
  fildes_t ret = {.type = stream, .perms = perms, .data = NULL};
  return ret;
}

int write_vio(fildes_t *fildes, uint32_t n, const void *src) {
  if (fildes->type == VIRT_STREAM_STDOUT ||
      fildes->type == VIRT_STREAM_STDERR) {
    display_str(src, n);
    return n;
  }
  return -1;
}

extern int keybuf_i;
extern char keybuf[64];
int read_vio(fildes_t *fildes, uint32_t n, void *ret) {
  if (fildes->type == VIRT_STREAM_STDIN) {
    uint32_t i = 0;
    // TODO: very much not thread-safe
    int tmp_keybuf_i = keybuf_i, keybuf_start = tmp_keybuf_i;
    while (i < n) {
      asm("hlt");
      if (keybuf_i > tmp_keybuf_i) {
        i++;
        tmp_keybuf_i = keybuf_i;
        printf("Got a key!\n");
      }
    }
    printf("Ret: %p\n", ret);
    memcpy(keybuf + keybuf_start, ret, n);
    keybuf_i = keybuf_start;
    return n;
  }
  return -1;
}

// stub as we cannot close these streams
void close_vio(fildes_t *fildes) {}
