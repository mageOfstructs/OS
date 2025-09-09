#ifndef VM_H

#define VM_H

#include <stdbool.h>
#include <stdint.h>

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
typedef struct pde {
  uint8_t flags;
  uint8_t avladdr; // lower 4 bits are avl, higher ones are bits 12-15 of addr
  uint16_t haddr;  // bits 16-31 of addr
} __attribute__((packed)) pde_t;

#define BLANK_PDE 0x00000002
#define BLANK_PTE BLANK_PDE

#define PG_SIZE 4096

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
typedef struct pte {
  uint8_t flags;
  uint8_t g : 1;
  uint8_t avl : 3;
  uint8_t laddr : 4; // bits 12-15 of addr
  uint16_t haddr;    // bits 16-31 of addr
} __attribute__((packed)) pte_t;

void setup_vm(void);
int vm_map(uint32_t vaddr_start, uint32_t len);
int vm_unmap(uint32_t vaddr_start, uint32_t len);
int vm_map_ext(uint32_t vaddr, uint32_t len, uint32_t *old, uint32_t *n,
               bool writable, bool user);
int vm_chk_map(uint32_t vaddr);
void *get_physaddr(void *virtualaddr);

#endif // !VM_H
