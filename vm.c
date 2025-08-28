#include "vm.h"
#include "printf.h"

uint32_t page_dir[1024] __attribute__((aligned(4096)));
uint32_t kernel_pt[1024] __attribute__((aligned(4096)));

void fill_pde(pde_t *p, uint32_t addr, bool rw, bool us) {
  p->flags = 1; // we'll do swap later
  if (rw)
   p->flags |= 2;
  if (us)
   p->flags |= 4;
  p->avl = 0;
  p->laddr = (uint16_t)(addr >> 12);
  p->haddr = (uint8_t)(addr >> 28);
}

void *get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long *pd = page_dir;
    // Here you need to check whether the PD entry is present.
    printf("pdindex: %d\n", pdindex);
    printf("ptindex: %d\n", ptindex);

    unsigned long *pt = pd[pdindex] & ~0xFFF;
    printf("%p == %p?\n", pt, kernel_pt);
    // Here you need to check whether the PT entry is present.
    
    printf("PTE: %p\n", (pt[ptindex]));
    printf("Page: %p\n", (pt[ptindex] & ~0xFFF));

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

void enable_paging(void) {
  printf("page_dir addr is %p\n", page_dir);
  printf("physaddr of 0x1000 is: %p", get_physaddr((void*)0x1000));
  printf("physaddr of 0x100f is: %p", get_physaddr((void*)0x100F));
  printf("physaddr of 0x5000 is: %p", get_physaddr((void*)0x5000));
  asm volatile ("mov eax, %0\n\t"
                "mov cr3, eax\n\t"
                "mov eax, cr0\n\t"
                "or eax, 0x80000001\n\t"
                "mov cr0, eax\n\t" :: "gm"(page_dir)); // okay so I have no idea what/why just happened but I added the g here and now it works?????
  // for (;;) asm("hlt");
}

// void setup_vm(void) {
//   for (int i = 0; i < 1024; i++)
//     page_dir[i] = BLANK_PDE;
//   // map kernel to exactly where it currently is
//   for (int i = 0; i < 1024; i++) {
//     kernel_pt[i] = (i * 0x1000) | 3;
//   }
//   page_dir[1] = ((unsigned int)kernel_pt) | 3;
//
//   enable_paging();
//   for (;;) asm("hlt");
// }

void setup_vm(void) {
  //set each entry to not present
  unsigned int i;
  for(i = 0; i < 1024; i++)
  {
      // This sets the following flags to the pages:
      //   Supervisor: Only kernel-mode can access them
      //   Write Enabled: It can be both read from and written to
      //   Not Present: The page table is not present
      page_dir[i] = 0x00000002;
  }

  // holds the physical address where we want to start mapping these pages to.
  // in this case, we want to map these pages to the very beginning of memory.

  //we will fill all 1024 entries in the table, mapping 4 megabytes
  for(i = 0; i < 1024; i++)
  {
      // As the address is page aligned, it will always leave 12 bits zeroed.
      // Those bits are used by the attributes ;)
      kernel_pt[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
  }
  page_dir[0] = ((unsigned long)kernel_pt) | 3;
  enable_paging();
}
