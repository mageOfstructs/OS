#include <stdint.h>
#include <stdbool.h>

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
  uint8_t avl: 4;
  uint16_t laddr; // bits 12-27 of addr
  uint8_t haddr : 4; // bits 28-31 of addr
} __attribute__((packed)) pde_t;

#define BLANK_PDE 0x00000002

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
  uint16_t laddr; // bits 12-27 of addr
  uint8_t haddr : 4; // bits 28-31 of addr
} __attribute__((packed)) pte_t;
