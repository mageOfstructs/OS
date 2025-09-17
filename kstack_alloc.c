#include "kstack_alloc.h"
#include "binops.h"
#include "log.h"

static uint32_t cur_stack = MIN_STACK;
static uint8_t stack_usage_bitmap[(MAX_STACK - MIN_STACK) /
                                  (KSTACK_SIZE + sizeof(uint8_t) * 8)];

static inline uint32_t __get_stack_i(uint32_t stack) {
  return ((stack & ~0xFFF) - MIN_STACK) / KSTACK_SIZE;
}

uint32_t get_next_stack() {
  uint32_t ret = cur_stack + 0x1000;
  log("New kernel stack: %p\n", ret);
  set_bit(stack_usage_bitmap, __get_stack_i(ret));
  cur_stack += KSTACK_SIZE;
  if (cur_stack > MAX_STACK) {
    cur_stack = MIN_STACK;
    while (get_bit(stack_usage_bitmap, __get_stack_i(cur_stack)) &&
           cur_stack <= MAX_STACK)
      cur_stack += KSTACK_SIZE;
    if (cur_stack > MAX_STACK) {
      err("ran out of stacks!");
      asm("cli; hlt_loop: hlt; jmp hlt_loop");
    }
  }
  return ret;
}

void release_stack(uint32_t stack) {
  clear_bit(stack_usage_bitmap, __get_stack_i(stack));
}
