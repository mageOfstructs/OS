#include "vm.h"
#include "phys_alloc.h"
#include "printf.h"
#include "utils.h"
#include <stdint.h>

static uint32_t page_dir[1024] __attribute__((aligned(4096)));
static uint32_t kernel_pt[1024] __attribute__((aligned(4096)));

// static uint32_t pt_space_pt[1024] __attribute__((aligned(4096)));
static uint32_t *pt_space_pt = (uint32_t *)(0x0);
static uint32_t *pt_space =
    (uint32_t *)(0x400000); // note: need to increase it by 1024 to access the
void *freelist_buf = (void *)(0x400000 + 126 * 1024 * 4); // wrong calc
// next table
static uint8_t free_pts_bitmap[128] = {
    255, [127] = 0b00111111}; // we already have two page tables, so there's no
                              // need for allocating that space again here

void fill_pde(pde_t *p, uint32_t addr, bool write_allowed,
              bool available_to_userspace) {
  p->flags = 1; // we'll do swap later
  if (write_allowed)
    p->flags |= 2;
  if (available_to_userspace)
    p->flags |= 4;
  p->avladdr = (uint8_t)(addr >> 8);
  p->haddr = (uint16_t)(addr >> 16);
}

void fill_pte(pte_t *p, uint32_t addr, bool write_allowed,
              bool available_to_userspace, bool global) {
  p->flags = 1;
  if (write_allowed)
    p->flags |= 2;
  if (available_to_userspace)
    p->flags |= 4;
  p->g = global & 1; // make sure we don't overflow the bit field
  p->avl = 0;
  printf("fill_pte physaddr: %p %p;", p, addr);
  p->laddr = (uint8_t)(addr >> 12) & 0x0F;
  p->haddr = (uint16_t)(addr >> 16);
}

void *get_physaddr(void *virtualaddr) {
  unsigned long pdindex = (unsigned long)virtualaddr >> 22;
  unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

  unsigned long *pd = page_dir;

  unsigned long *pt = (uint32_t *)(pd[pdindex] & ~0xFFF);
  printf("pd_i: %d, pt_i: %d PT: %p ", pdindex, ptindex, pt);

  printf("PTE: %p %p", &(pt[ptindex]), (pt[ptindex]));
  printf("Page: %p\n", (pt[ptindex] & ~0xFFF));

  return (void *)((pt[ptindex] & ~0xFFF) +
                  ((unsigned long)virtualaddr & 0xFFF));
}

static inline bool is_pde_present(uint32_t i) { return page_dir[i] & 1; }

static inline bool is_pte_present(uint32_t *pt, uint32_t i) {
  return pt[i] & 1;
}

static inline uint32_t get_addr(uint32_t entry) { return entry & ~0xFFF; }

uint32_t *alloc_pt() {
  for (int i = 0; i < 1024; i++) {
    if ((free_pts_bitmap[i / 8] >> (i % 8)) & 1) {
      free_pts_bitmap[i / 8] |= 1 << (i % 8);
      for (int j = 0; j < 1024; j++) {
        pt_space[i * 1024 + j] = BLANK_PTE;
      }
      return &pt_space[i * 1024];
    }
  }
  return NULL;
}
int free_pt(uint32_t *pt) {
  if (pt < pt_space || pt >= (pt_space + 1024 * 1024))
    return -1; // out of bounds
  uint32_t i = (uint32_t)(pt - pt_space) / 1024;
  if (free_pts_bitmap[i / 8] >> i % 8) { // did we allocate a page table there?
    free_pts_bitmap[i / 8] &= ~(1 << (i % 8));
    return 0; // success
  } else
    return -2; // trying to free a deallocated page
}

/**
 * helper method for vm mapping
 * maps n pages, starting from vaddr_start
 * As addresses must be aligned to 4K,
 *the lower 12 bits of vaddr_start are ignored will fail if address is already
 *mapped, therefore it doesn't do a TLB flush
 **/
int vm_map(uint32_t vaddr_start, uint32_t len) {
  uint32_t pd_i = vaddr_start >> 22;
  uint32_t pt_i = vaddr_start >> 12 & 0x03FF;

  uint32_t *pt;
  while (len > 0) {
    if (is_pde_present(pd_i)) {
      pt = (uint32_t *)(page_dir[pd_i] & ~0xFFF);
    } else {
      pt = alloc_pt();
      if (!pt)
        return 2; // ran out of pt_space
      fill_pde((pde_t *)&page_dir[pd_i], (uint32_t)pt, true, false);
      printf("vm_map: %p == %p? \n", (uint32_t)pt, page_dir[pd_i]);
    }
    while (len > 0 && pt_i < 1024) {
      if (is_pte_present(pt, pt_i)) // address is already mapped
        return 1;

      uint32_t paddr = (uint32_t)phys_alloc(1);
      KASSERT(paddr);
      printf("pt: %p\n", pt);
      fill_pte((pte_t *)&pt[pt_i++], paddr, true, false, false);
      printf(" pd_i: %d, pt_i: %d, Pte: %p", pd_i, pt_i - 1,
             ((uint32_t *)page_dir[pd_i])[pt_i - 1]);
      len--;
    }
    pd_i++;
    pt_i = 0;
  }
  return 0; // success
}

static inline void __native_flush_tlb_single(unsigned long addr) {
  asm volatile("invlpg [%0]" ::"r"(addr) : "memory");
}

static inline void __tlb_flush() {
  asm volatile("mov eax, cr3\n\t"
               "mov cr3, eax");
}

int vm_unmap(uint32_t vaddr_start, uint32_t len) {
  uint32_t pd_i = vaddr_start >> 22;
  uint32_t pt_i = vaddr_start >> 12 & 0x03FF;
  uint32_t *pt;
  int ret = 0;
  printf("pd_i: %d; pt_i: %d\n", pd_i, pt_i);

  while (len > 0) {
    if (!is_pde_present(pd_i))
      return 1; // encountered non-present pde
    pt = (uint32_t *)get_addr(page_dir[pd_i]);
    while (len > 0 && pt_i < 1024) {
      if (!is_pte_present(pt, pt_i)) {
        printf("ERR: %p is not present!", pt[pt_i]);
        return 2; // encountered non-present pte
      }
      // printf("vm_unmap: Unmapping page %p\n", get_addr(pt[pt_i]));
      // __native_flush_tlb_single(get_addr(pt[pt_i]));
      phys_dealloc((void *)(pt[pt_i] & ~0xFFF));
      printf("vm_unmap: Unmapping page %p\n", vaddr_start);
      __native_flush_tlb_single(vaddr_start);
      vaddr_start += 4096;
      pt[pt_i++] = BLANK_PTE;
      len--;
    }
    pd_i++;
    pt_i = 0;
  }
  return 0;
}

void enable_paging(void) {
  printf("page_dir addr is %p\n", page_dir);
  // printf("physaddr of 0x1000 is: %p\n", get_physaddr((void *)0x1000));
  // printf("physaddr of 0x100f is: %p\n", get_physaddr((void *)0x100F));
  // printf("physaddr of 0x4000 is: %p\n", get_physaddr((void *)0x4000));
  // printf("physaddr of 0x4800 is: %p\n", get_physaddr((void *)0x4800));
  // printf("physaddr of 0x1410 is: %p\n", get_physaddr((void *)0x1410));

  asm volatile("mov eax, %0\n\t"
               "mov cr3, eax\n\t"
               "mov eax, cr0\n\t"
               "or eax, 0x80000001\n\t"
               "mov cr0, eax\n\t" ::"g"(
                   page_dir)); // okay so I have no idea what/why just happened
                               // but I added the g here and now it works?????
  // printf("physaddr of 0x400000 is: %p\n", get_physaddr((void *)0x400000));

  vm_map(0x12345678, 1);
  vm_map(0x12347678, 1);
  vm_unmap(0x12345678, 1);
  int map_status = vm_map(0x12345678, 2);
  printf("vm_map status: %d\n", map_status);
  printf("physaddr of 0x12345678 is: %p\n", get_physaddr((void *)0x12345678));
  printf(" done %p %p\n", &(((uint32_t *)page_dir[72])[837]),
         ((uint32_t *)page_dir[72])[837]);
  printf("physaddr of 0x12346678 is: %p\n", get_physaddr((void *)0x12346678));
  printf("physaddr of 0x12347678 is: %p\n", get_physaddr((void *)0x12347678));

  vm_unmap(0x12347678, 1);
  // printf("%c", *((char *)0x12347678));
  // printf("%c", *((char *)0x12346678));
  //
  // printf("\n%d\n", vm_unmap(0x12345678, 2));
  // printf("%c", *((char *)0x12346678));
}

void setup_vm(void) {
  // set each entry to not present
  unsigned int i;
  for (i = 0; i < 1024; i++) {
    // This sets the following flags to the pages:
    //   Supervisor: Only kernel-mode can access them
    //   Write Enabled: It can be both read from and written to
    //   Not Present: The page table is not present
    page_dir[i] = 0x00000002;
  }

  // we will fill all 1024 entries in the table, mapping 4 megabytes
  for (i = 0; i < 1024; i++) {
    // As the address is page aligned, it will always leave 12 bits zeroed.
    // Those bits are used by the attributes ;)
    kernel_pt[i] =
        (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.

    pt_space_pt[i] = (uint32_t)(((i * 0x1000 + (uint32_t)pt_space)) | 3);
  }
  fill_pde((pde_t *)page_dir, (unsigned long)kernel_pt, true, false);
  fill_pde((pde_t *)(&page_dir[1]), (unsigned long)pt_space_pt, true, false);

  init_physalloc(0x00800000, 0x00801000);
  enable_paging();
}
