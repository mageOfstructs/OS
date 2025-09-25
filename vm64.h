#ifndef VM64_H

#define VM64_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define BLANK_PDE 0x00000002
#define BLANK_PTE BLANK_PDE

#define PG_SIZE 4096

#define ENTRY_TYPE uint64_t

/*
 * flags:
 * Present
 * R/W
 * U/S (set to 1 for usermode pages)
 * PWT (set to 0 for now)
 * PCD (set to 0 for now)
 * A (set to 0)
 * AVL
 * PS (set to 0)
 */
typedef struct pde64 {
  uint8_t flags;
  uint8_t avl : 4;
  uint8_t laddr : 4;   // bits 12-15 of addr
  uint16_t maddr;      // bits 16-31 of addr
  uint32_t haddr : 20; // NOTE: need to know M for this
  uint32_t reserved : 12;
} __attribute__((packed)) pde64_t;

typedef pde64_t pdpt_t;
typedef pde64_t pml4_t;

/*
 * flags:
 * Present
 * R/W
 * U/S (set to 1 for usermode pages)
 * PWT (set to 0 for now)
 * PCD (set to 0 for now)
 * A (set to 0)
 * D (set to 0)
 * PAT (set to 0 for now)
 */
typedef struct pte64 {
  uint8_t flags;
  uint8_t g : 1;
  uint8_t avl : 3;
  uint8_t laddr : 4; // bits 12-15 of addr
  uint16_t maddr;    // bits 16-31 of addr
  uint32_t haddr : 20;
  uint32_t reserved : 12;
} __attribute__((packed)) pte64_t;

void setup_vm(void);
int vm_map64(uint64_t vaddr_start, uint64_t len);
int vm_unmap64(uint64_t vaddr_start, uint64_t len);
int vm_map_ext64(uint64_t vaddr, uint64_t len, uint64_t *old, uint64_t *n,
                 bool writable, bool user);
int vm_chk_map64(uint64_t vaddr);
int vm_map_buf64(void *buf, size_t sz, bool writable, bool user);
void vm_unmap_buf64(void *buf, size_t sz);
void *get_physaddr64(void *virtualaddr);

#endif // !VM_H
