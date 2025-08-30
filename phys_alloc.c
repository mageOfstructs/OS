#include "phys_alloc.h"
#include "llist.h"
#include "malloc.h"
#include "printf.h"
#include "utils.h"
#include <stdint.h>

LIST_DEF(free_block_t, fb)
extern llist_t_fb *freelist_buf;

static void *freelist_nodes;
static alloc_t alloc;

static void *alloc_node() { return alloc_ctx(&alloc, sizeof(llist_node_fb)); }

void dbg_llist() {
  int i = 0;
  llist_node_fb *cur = freelist_buf->head;
  while (cur && i < 3) {
    printf("%d ", cur->val.sz);
    cur = cur->next;
    i++;
  }
  printf("\n");
}

void init_physalloc(uint32_t heap_start, uint32_t heap_end) {
  freelist_nodes = (llist_node_fb *)(freelist_buf + 1);

  llist_t_fb freelist = LIST_INIT;
  free_block_t init_fb = {.start = (void *)heap_start,
                          .sz = (heap_end - heap_start)};
  alloc = init_alloc(freelist_nodes, 1024);

  llist_node_fb *node_space = alloc_node();
  KASSERT(node_space);

  llist_node_fb node = NODE_INIT(init_fb);
  *node_space = node;

  llist_pushb_fb(&freelist, node_space);

  *freelist_buf = freelist;
  printf("init_physalloc: range %p-%p\n", heap_start, heap_end);
}

// TODO: have some logic that combines different free_blocks if they're adjacent
void llist_fb_sorted_insert(llist_node_fb *n) {

  if (!freelist_buf->head || (n->val.sz >= freelist_buf->head->val.sz)) {
    printf("llist_fb_sorted_insert: %p %d >= %d\n", freelist_buf->head,
           n->val.sz, freelist_buf->head->val.sz);
    llist_pushf_fb(freelist_buf, n);
    return;
  }
  if (!freelist_buf->tail || n->val.sz <= freelist_buf->tail->val.sz) {
    llist_pushb_fb(freelist_buf, n);
    return;
  }

  llist_node_fb **cur;
  if (n->val.sz <= MAX_SMALL_PG_ALLOC_SZ) {
    cur = &freelist_buf->tail;
    while (*cur && (*cur)->val.sz > n->val.sz) {
      cur = &(*cur)->prev;
    }
    n->next = (*cur)->next;
    n->prev = *cur;
    if ((*cur)->next)
      (*cur)->next->prev = n;
    (*cur)->next = n;
  } else {
    llist_node_fb **cur = &freelist_buf->head;
    while (*cur && (*cur)->val.sz < n->val.sz) {
      cur = &(*cur)->next;
    }
    n->prev = (*cur)->prev;
    n->next = *cur;
    if ((*cur)->prev)
      (*cur)->prev->next = n;
    (*cur)->prev = n;
  }
  freelist_buf->sz++;
}

void *phys_alloc(uint16_t n) {
  printf("phys_alloc: trying to allocate %d page(s)\n", n);
  int off;
  llist_node_fb *cur;
  if (n <= MAX_SMALL_PG_ALLOC_SZ) {
    off = -1;
    cur = freelist_buf->tail;
  } else {
    off = 1;
    cur = freelist_buf->head;
  }

  while (cur) {
    printf("Trying free block with %d pages\n", cur->val.sz);
    if (cur->val.sz >= n) {
      llist_node_fb_remove(freelist_buf, cur);
      freelist_buf->sz--;
      if (freelist_buf->head)
        dbg_llist();
      // if (freelist_buf->tail)
      //   printf("tail: %d\n", freelist_buf->tail->val.sz);
      // else
      //   printf("tail empty\n");
      if (cur->val.sz > n) {
        free_block_t remaining = {.start = cur->val.start + n * 4096,
                                  .sz = cur->val.sz - n};
        llist_node_fb remaining_n = NODE_INIT(remaining);
        llist_node_fb *remaining_ptr = alloc_node();
        KASSERT(remaining_ptr);
        *remaining_ptr = remaining_n;
        llist_fb_sorted_insert(remaining_ptr);
      }
      if (freelist_buf->head)
        printf("head: %d\n", freelist_buf->head->val.sz);
      else
        printf("head empty\n");

      void *ret = (void *)((uint32_t)cur->val.start | (uint32_t)(n - 1));
      printf("phys_alloc: allocated page %p\n", ret);
      return ret;
    }
    cur = llist_node_fb_offset(cur, off);
  }
  return NULL;
}

void phys_dealloc(void *allocation) {
  uint32_t phys_alloc = (uint32_t)allocation;
  void *start = (void *)(phys_alloc & ~0xFFF);
  uint32_t size = (phys_alloc & 0xFFF) + 1;
  printf("Creating new free block at %p with %d pages\n", start, size);
  free_block_t new_free = {.start = start, .sz = size};
  llist_node_fb new_free_n = NODE_INIT(new_free);
  llist_node_fb *new_free_nspace = alloc_node(); // FIXME: uhhh tis broken
  KASSERT(new_free_nspace);
  if (freelist_buf->head)
    printf("head before: %d\n", freelist_buf->head->val.sz);
  else
    printf("head before empty\n");
  *new_free_nspace = new_free_n;
  llist_fb_sorted_insert(new_free_nspace);
  if (freelist_buf->head)
    printf("head: %d\n", freelist_buf->head->val.sz);
  else
    printf("head empty\n");
}
