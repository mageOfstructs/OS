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
static uint32_t phys_heap_start;

static void *alloc_node() { return alloc_ctx(&alloc, sizeof(llist_node_fb)); }

void dbg_llist() {
  int i = 0;
  printf("dbg_llist: ");
  llist_node_fb *cur = freelist_buf->head;
  while (cur && i < 20) {
    printf("[%p %d] ", cur->val.start, cur->val.sz);
    cur = cur->next;
    i++;
  }
  printf("\n");
}

void dbg_llist_rev() {
  int i = 0;
  printf("dbg_llist_rev: ");
  llist_node_fb *cur = freelist_buf->tail;
  while (cur && i < 20) {
    printf("[%p %d] ", cur->val.start, cur->val.sz);
    cur = cur->prev;
    i++;
  }
  printf("\n");
}

void init_physalloc(uint32_t heap_start, uint32_t heap_end) {
  phys_heap_start = heap_start;
  freelist_nodes = (llist_node_fb *)(freelist_buf + 1);

  llist_t_fb freelist = LIST_INIT;
  free_block_t init_fb = {.start = (void *)heap_start,
                          .page_off = 0,
                          .sz = (heap_end - heap_start) / 4096};
  alloc = init_alloc(freelist_nodes, 2048 - sizeof(llist_t_fb));

  llist_node_fb *node_space = alloc_node();
  KASSERT(node_space);

  llist_node_fb node = NODE_INIT(init_fb);
  *node_space = node;

  llist_pushb_fb(&freelist, node_space);

  *freelist_buf = freelist;
  printf("init_physalloc: range %p-%p (%d pages)\n", heap_start, heap_end,
         init_fb.sz);
}

void llist_fb_sorted_insert(llist_node_fb *n);

int combine_fbs(llist_node_fb *cur, llist_node_fb *n) {
  if (cur->val.page_off == n->val.page_off - cur->val.sz) {
    printf("a %p %d , %p %d\n", cur->val.start, cur->val.sz, n->val.start,
           n->val.sz);
    cur->val.sz += n->val.sz;
    llist_node_fb_remove(freelist_buf, cur);
    // dbg_llist_rev();
    llist_fb_sorted_insert(cur);
    dbg_llist_rev();
    return 1;
  } else if (cur->val.page_off == n->val.page_off + n->val.sz) {
    cur->val.start = (void *)((uint32_t)n->val.start & ~0xFFF);
    cur->val.page_off = n->val.page_off;
    cur->val.sz += n->val.sz;
    llist_node_fb_remove(freelist_buf, cur);
    // dbg_llist_rev();
    llist_fb_sorted_insert(cur);
    printf("b ");
    // dbg_llist_rev();
    return 1;
  }
  return 0;
}

void llist_fb_sorted_insert(llist_node_fb *n) {

  if (!freelist_buf->head || (n->val.sz >= freelist_buf->head->val.sz)) {
    if (combine_fbs(freelist_buf->head, n))
      return;

    llist_pushf_fb(freelist_buf, n);
    // dbg_llist();
    // dbg_llist_rev();
    return;
  }
  if (!freelist_buf->tail || n->val.sz <= freelist_buf->tail->val.sz) {
    if (combine_fbs(freelist_buf->tail, n))
      return;
    printf("1!");
    llist_pushb_fb(freelist_buf, n);
    return;
  }

  llist_node_fb **cur;
  if (n->val.sz <= MAX_SMALL_PG_ALLOC_SZ) {
    cur = &freelist_buf->tail;
    while (*cur && (*cur)->val.sz > n->val.sz) {
      // printf("%p %p\n", (*cur)->val.start, n->val.start);
      if (combine_fbs(*cur, n))
        return;
      cur = &(*cur)->prev;
    }
    printf("2!");
    n->next = (*cur)->next;
    n->prev = *cur;
    if ((*cur)->next)
      (*cur)->next->prev = n;
    (*cur)->next = n;
  } else {
    printf("3!");
    cur = &freelist_buf->head;
    while (*cur && (*cur)->val.sz < n->val.sz) {
      if (combine_fbs(*cur, n))
        return;
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
  printf("phys_alloc: trying to allocate %d page(s) %d\n", n, freelist_buf->sz);
  // dbg_llist();
  // dbg_llist_rev();
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
    // printf("Trying free block with %d pages\n", cur->val.sz);
    if (cur->val.sz >= n) {
      llist_node_fb_remove(freelist_buf, cur);
      // if (freelist_buf->head)
      //   dbg_llist();
      // if (freelist_buf->tail)
      //   printf("tail: %d\n", freelist_buf->tail->val.sz);
      // else
      //   printf("tail empty\n");
      if (cur->val.sz > n) {
        free_block_t remaining = {.start = cur->val.start + n * 4096,
                                  .page_off = cur->val.page_off + n,
                                  .sz = cur->val.sz - n};
        llist_node_fb remaining_n = NODE_INIT(remaining);
        llist_node_fb *remaining_ptr = alloc_node();
        KASSERT(remaining_ptr);
        *remaining_ptr = remaining_n;
        llist_fb_sorted_insert(remaining_ptr);
      }
      // if (freelist_buf->head)
      //   printf("head: %d\n", freelist_buf->head->val.sz);
      // else
      //   printf("head empty\n");

      void *ret = (void *)((uint32_t)cur->val.start | (uint32_t)(n - 1));
      KASSERT(dealloc_ctx(&alloc, cur, sizeof(llist_node_fb)) == 0);
      dbg_llist();
      dbg_llist_rev();
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
  free_block_t new_free = {.start = start,
                           .page_off =
                               ((uint32_t)start - phys_heap_start) / 4096,
                           .sz = size};
  llist_node_fb new_free_n = NODE_INIT(new_free);
  llist_node_fb *new_free_nspace = alloc_node();
  KASSERT(new_free_nspace);
  // if (freelist_buf->head)
  //   printf("head before: %d\n", freelist_buf->head->val.sz);
  // else
  //   printf("head before empty\n");
  *new_free_nspace = new_free_n;
  llist_fb_sorted_insert(new_free_nspace);
  dbg_llist();
  dbg_llist_rev();
  // if (freelist_buf->head)
  //   printf("head: %d\n", freelist_buf->head->val.sz);
  // else
  //   printf("head empty\n");
}
