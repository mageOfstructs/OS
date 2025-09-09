#include "vm.h"
#include <stddef.h>
#include <stdint.h>

#define LIST_INIT {.head = NULL, .tail = NULL, .sz = 0}
#define NODE_INIT(v) {.next = NULL, .prev = NULL, .val = v}

#define LIST_DEF(type, canon_name)                                             \
  typedef struct node {                                                        \
    struct node *next;                                                         \
    struct node *prev;                                                         \
    type val;                                                                  \
  } llist_node_##canon_name;                                                   \
                                                                               \
  typedef struct llist_##canon_name {                                          \
    llist_node_##canon_name *head;                                             \
    llist_node_##canon_name *tail;                                             \
    size_t sz;                                                                 \
  } llist_t_##canon_name;                                                      \
                                                                               \
  llist_node_##canon_name *llist_get_##canon_name(llist_t_##canon_name *l,     \
                                                  uint32_t i) {                \
    if (i >= l->sz)                                                            \
      return NULL;                                                             \
    llist_node_##canon_name *cur;                                              \
    if (i < l->sz / 2) {                                                       \
      cur = l->head;                                                           \
      while (i && cur) {                                                       \
        cur = cur->next;                                                       \
        i--;                                                                   \
      }                                                                        \
    } else {                                                                   \
      cur = l->tail;                                                           \
      i = l->sz - i;                                                           \
      while (i && cur) {                                                       \
        cur = cur->prev;                                                       \
        i--;                                                                   \
      }                                                                        \
    }                                                                          \
    return cur;                                                                \
  }                                                                            \
                                                                               \
  int llist_insert_##canon_name(llist_t_##canon_name *l,                       \
                                llist_node_##canon_name *nnode, uint32_t i) {  \
    llist_node_##canon_name *cur_node_at_i = llist_get_##canon_name(l, i);     \
    if (!cur_node_at_i)                                                        \
      return 0;                                                                \
    nnode->prev = cur_node_at_i->prev;                                         \
    if (cur_node_at_i->prev)                                                   \
      cur_node_at_i->prev->next = nnode;                                       \
    cur_node_at_i->prev = nnode;                                               \
    nnode->next = cur_node_at_i;                                               \
    l->sz++;                                                                   \
    return l->sz;                                                              \
  }                                                                            \
                                                                               \
  void llist_popf_##canon_name(llist_t_##canon_name *l,                        \
                               llist_node_##canon_name *ret) {                 \
    ret = l->head;                                                             \
    if (ret) {                                                                 \
      l->head = ret->next;                                                     \
      if (l->head)                                                             \
        l->head->prev = NULL;                                                  \
      if (l->sz == 1)                                                          \
        l->tail = NULL;                                                        \
      l->sz--;                                                                 \
    }                                                                          \
  }                                                                            \
                                                                               \
  void llist_popb_##canon_name(llist_t_##canon_name *l,                        \
                               llist_node_##canon_name *ret) {                 \
    ret = l->tail;                                                             \
    if (ret) {                                                                 \
      l->tail = ret->prev;                                                     \
      if (l->tail)                                                             \
        l->tail->next = NULL;                                                  \
      if (l->sz == 1)                                                          \
        l->head = NULL;                                                        \
      l->sz--;                                                                 \
    }                                                                          \
  }                                                                            \
                                                                               \
  void llist_pushb_##canon_name(llist_t_##canon_name *l,                       \
                                llist_node_##canon_name *nnode) {              \
    if (l->tail)                                                               \
      l->tail->next = nnode;                                                   \
    nnode->prev = l->tail;                                                     \
    l->tail = nnode;                                                           \
    if (!l->sz)                                                                \
      l->head = nnode;                                                         \
    l->sz++;                                                                   \
  }                                                                            \
                                                                               \
  void llist_pushf_##canon_name(llist_t_##canon_name *l,                       \
                                llist_node_##canon_name *nnode) {              \
    if (l->head)                                                               \
      l->head->prev = nnode;                                                   \
    nnode->next = l->head;                                                     \
    l->head = nnode;                                                           \
    if (!l->sz)                                                                \
      l->tail = nnode;                                                         \
    l->sz++;                                                                   \
  }                                                                            \
  void llist_node_##canon_name##_remove(llist_t_##canon_name *l,               \
                                        llist_node_##canon_name *n) {          \
    if (n->prev)                                                               \
      n->prev->next = n->next;                                                 \
    else                                                                       \
      l->head = n->next;                                                       \
    if (n->next)                                                               \
      n->next->prev = n->prev;                                                 \
    else                                                                       \
      l->tail = n->prev;                                                       \
    l->sz--;                                                                   \
  }                                                                            \
  llist_node_##canon_name *llist_node_##canon_name##_offset(                   \
      llist_node_##canon_name *n, int off) {                                   \
    while (off != 0) {                                                         \
      if (off < 0) {                                                           \
        n = n->prev;                                                           \
        off++;                                                                 \
      } else {                                                                 \
        n = n->next;                                                           \
        off--;                                                                 \
      }                                                                        \
    }                                                                          \
    return n;                                                                  \
  }
