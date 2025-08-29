#ifndef VM_H

#define VM_H

#include <stdint.h>
#include <stdbool.h>

#define NULL 0

void setup_vm(void);

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
  uint16_t haddr; // bits 16-31 of addr
} __attribute__((packed)) pde_t;

#define BLANK_PDE 0x00000002
#define BLANK_PTE BLANK_PDE

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
  uint8_t avl: 3;
  uint8_t laddr : 4; // bits 12-15 of addr
  uint16_t haddr; // bits 16-31 of addr
} __attribute__((packed)) pte_t;

#endif // !VM_H
