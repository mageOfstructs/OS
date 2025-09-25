#include "vm64.h"
#include "binops.h"
#include "idt.h"
#include "kstack_alloc.h"
#include "log.h"
#include "malloc.h"
#include "phys_alloc.h"
#include "printf.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <cpuid.h>

static ENTRY_TYPE M;

static ENTRY_TYPE page_dir[512] __attribute__((aligned(4096)));
static ENTRY_TYPE zero_page_dir[512] __attribute__((aligned(4096)));
static ENTRY_TYPE kernel_pt[512] __attribute__((aligned(4096)));

// static ENTRY_TYPE pt_space_pt[1024] __attribute__((aligned(4096)));
static ENTRY_TYPE *pt_space_pt = (ENTRY_TYPE *)(0x0);
static ENTRY_TYPE *pt_space =
    (ENTRY_TYPE *)(0x400000); // note: need to increase it by 1024 to access the
void *freelist_buf = (void *)(0x400000 + 126 * 1024 * 4); // wrong calc
// next table
static uint8_t free_pts_bitmap[128] = {
    255, [127] = 0b00111111}; // we already have two page tables, so there's no
                              // need for allocating that space again here

static inline ENTRY_TYPE __vaddr_get_pdei(ENTRY_TYPE vaddr) {
  return (vaddr >> 22 & 0x3FF);
}
static inline ENTRY_TYPE __vaddr_get_ptei(ENTRY_TYPE vaddr) {
  return (vaddr >> 12 & 0x3FF);
}
static inline ENTRY_TYPE __vaddr_get_poff(ENTRY_TYPE vaddr) {
  return (vaddr & 0x3FF);
}
static inline bool __entry_present(ENTRY_TYPE entry) { return entry & 1; }

static inline void __native_flush_tlb_single(ENTRY_TYPE addr) {
  asm volatile("invlpg [%0]" ::"r"(addr) : "memory");
}

static inline void __tlb_flush() {
  asm volatile("mov eax, cr3\n\t"
               "mov cr3, eax");
}

// TODO:
static void init_M() {}

void fill_pde64(pde64_t *p, ENTRY_TYPE addr, bool write_allowed,
                bool available_to_userspace) {
  p->flags = 1; // we'll do swap later
  if (write_allowed)
    p->flags |= 2;
  if (available_to_userspace)
    p->flags |= 4;
  p->avl = 0;
  p->haddr = (uint8_t)(addr >> 12);
  p->maddr = (uint16_t)(addr >> 16);
  p->haddr = (ENTRY_TYPE)(addr >> 32);
  p->reserved = 0;
  log("fill_pde64: pg[%p] = %p\n",
      ((size_t)p - (size_t)page_dir) / sizeof(ENTRY_TYPE), *p);
}

void fill_pte64(pte64_t *p, ENTRY_TYPE addr, bool write_allowed,
                bool available_to_userspace, bool global) {
  p->flags = 1;
  if (write_allowed)
    p->flags |= 2;
  if (available_to_userspace)
    p->flags |= 4;
  p->g = global & 1; // make sure we don't overflow the bit field
  p->avl = 0;
  printf("fill_pte64 physaddr: %p %p;", *p, addr);
  p->laddr = (uint8_t)(addr >> 12) & 0x0F;
  p->maddr = (uint16_t)(addr >> 16);
  p->haddr = (ENTRY_TYPE)(addr >> 32);
  p->reserved = 0;
}

void *get_physaddr64(void *virtualaddr) {
  unsigned long pdindex = (unsigned long)virtualaddr >> 22;
  unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

  unsigned long *pd = page_dir;

  printf("PDE: %p\n", pd[pdindex]);
  unsigned long *pt = (ENTRY_TYPE *)(pd[pdindex] & ~0xFFF);
  printf("pd_i: %d, pt_i: %d PT: %p ", pdindex, ptindex, pt);

  printf("PTE: %p %p", &(pt[ptindex]), (pt[ptindex]));
  printf("Page: %p\n", (pt[ptindex] & ~0xFFF));

  return (void *)((pt[ptindex] & ~0xFFF) +
                  ((unsigned long)virtualaddr & 0xFFF));
}

static inline bool is_pde_present(ENTRY_TYPE i) {
  return __entry_present(page_dir[i]);
}

static inline bool is_pte_present(ENTRY_TYPE *pt, ENTRY_TYPE i) {
  return __entry_present(pt[i]);
}

static inline ENTRY_TYPE get_addr(ENTRY_TYPE entry) { return entry & ~0xFFF; }

pte64_t *get_pte(ENTRY_TYPE vaddr) {
  ENTRY_TYPE pde_i = (vaddr >> 22) & 0x3FF;
  ENTRY_TYPE pte_i = (vaddr >> 12) & 0x3FF;
  if (!is_pde_present(pde_i))
    return NULL;
  ENTRY_TYPE *pt = &page_dir[pde_i];
  if (!is_pte_present(pt, pte_i))
    return NULL;
  return (pte64_t *)&pt[pte_i];
}

ENTRY_TYPE *alloc_pt() {
  for (int i = 0; i < 1024; i++) {
    if (!get_bit(free_pts_bitmap, i)) {
      set_bit(free_pts_bitmap, i);
      for (int j = 0; j < 1024; j++) {
        pt_space[i * 1024 + j] = BLANK_PTE;
      }
      return &pt_space[i * 1024];
    }
  }
  return NULL;
}
int free_pt(ENTRY_TYPE *pt) {
  if (pt < pt_space || pt >= (pt_space + 1024 * 1024))
    return -1; // out of bounds
  ENTRY_TYPE i = (ENTRY_TYPE)(pt - pt_space) / 1024;
  if (free_pts_bitmap[i / 8] >> i % 8) { // did we allocate a page table there?
    free_pts_bitmap[i / 8] &= ~(1 << (i % 8));
    return 0; // success
  } else
    return -2; // trying to free a deallocated page table
}

/**
 * helper method for vm mapping
 * maps n pages, starting from vaddr_start
 * As addresses must be aligned to 4K,
 *the lower 12 bits of vaddr_start are ignored will fail if address is already
 *mapped, therefore it doesn't do a TLB flush
 **/
int vm_map(ENTRY_TYPE vaddr_start, ENTRY_TYPE len) {
  ENTRY_TYPE pd_i = vaddr_start >> 22;
  ENTRY_TYPE pt_i = vaddr_start >> 12 & 0x03FF;

  ENTRY_TYPE *pt;
  while (len > 0) {
    if (is_pde_present(pd_i)) {
      pt = (ENTRY_TYPE *)(page_dir[pd_i] & ~0xFFF);
    } else {
      pt = alloc_pt();
      if (!pt)
        return 2; // ran out of pt_space
      fill_pde64((pde64_t *)&page_dir[pd_i], (ENTRY_TYPE)pt, true, false);
      printf("vm_map: %p == %p? \n", (ENTRY_TYPE)pt, page_dir[pd_i]);
    }
    while (len > 0 && pt_i < 1024) {
      if (is_pte_present(pt, pt_i)) // address is already mapped
        return 1;

      ENTRY_TYPE paddr = (ENTRY_TYPE)phys_alloc(1);
      KASSERT(paddr);
      printf("pt: %p\n", pt);
      fill_pte64((pte64_t *)&pt[pt_i++], paddr, true, false, false);
      printf(" pd_i: %d, pt_i: %d, Pte: %p", pd_i, pt_i - 1,
             ((ENTRY_TYPE *)page_dir[pd_i])[pt_i - 1]);
      len--;
    }
    pd_i++;
    pt_i = 0;
  }
  return 0; // success
}

/**
 * map a virtual memory region extended, works the same as vm_map with these
 *additions:
 - if old is not NULL: store previous page table entries in old
 - if new is not NULL: use entries stored in new instead of creating new ones
 **/
int vm_map_ext(ENTRY_TYPE vaddr, ENTRY_TYPE len, ENTRY_TYPE *old,
               ENTRY_TYPE *new, bool writable, bool user) {
  ENTRY_TYPE pd_i = __vaddr_get_pdei(vaddr), pt_i = __vaddr_get_ptei(vaddr);
  ENTRY_TYPE pte_buf_i = 0;
  ENTRY_TYPE *pt = &page_dir[pd_i];
  while (len > 0) {
    if (is_pde_present(pd_i)) {
      if ((page_dir[pd_i] & 0x06) ^ 0x06) {
        page_dir[pd_i] |= (writable << 1) | (user << 2);
      }
      pt = (ENTRY_TYPE *)(page_dir[pd_i] & ~0xFFF);
    } else {
      pt = alloc_pt();
      if (!pt)
        return 2; // ran out of pt_space
      fill_pde64((pde64_t *)&page_dir[pd_i], (ENTRY_TYPE)pt, writable, user);
      printf("vm_map: %p == %p? \n", (ENTRY_TYPE)pt, page_dir[pd_i]);
    }
    while (len > 0 && pt_i < 1024) {
      if (old)
        old[pte_buf_i] = pt[pt_i];

      if (is_pte_present(pt, pt_i))
        __native_flush_tlb_single(((ENTRY_TYPE)pd_i) << 22 | ((ENTRY_TYPE)pt_i)
                                                                 << 12);
      if (!new) {
        ENTRY_TYPE paddr = (ENTRY_TYPE)phys_alloc(1);
        KASSERT(paddr);
        fill_pte64((pte64_t *)&pt[pt_i], paddr, writable, user, false);
      } else
        pt[pt_i] = new[pte_buf_i];
      pte_buf_i++;
      pt_i++;
      len--;
    }
    pd_i++;
    pt_i = 0;
  }
  return 0;
}

/**
 * allocates sz pages and places entries into buf
 **/
int vm_map_buf(void *buf, size_t sz, bool writable, bool user) {
  size_t paddr;
  for (ENTRY_TYPE i = 0; i < sz; i++) {
    paddr = (size_t)phys_alloc(1);
    if (!paddr) {
      size_t pte;
      for (int j = 0; j < i; i++) {
        pte = ((ENTRY_TYPE *)buf)[j];
        phys_dealloc((void *)(pte & ~0xFFF));
      }
      return -1;
    }
    KASSERT(paddr);
    fill_pte64((pte64_t *)buf + i, paddr, writable, user, false);
  }
  return 0;
}

void vm_unmap_buf(void *buf, size_t sz) {
  ENTRY_TYPE *entries = (ENTRY_TYPE *)buf;
  for (size_t i = 0; i < sz; i++) {
    size_t page_addr = entries[i] & ~0xFFF;
    phys_dealloc((void *)page_addr);
    __native_flush_tlb_single(page_addr);
  }
}

int vm_chk_map(ENTRY_TYPE vaddr) {
  ENTRY_TYPE pt_i = __vaddr_get_ptei(vaddr);
  pte64_t *pte = get_pte(vaddr);
  if (!pte)
    return -1;
  ENTRY_TYPE ret = 0;
  while ((pt_i + ret) < 1024 && pte[pt_i + ret].flags & 1) {
    ret++;
  }
  return ret;
}

int vm_unmap(ENTRY_TYPE vaddr_start, ENTRY_TYPE len) {
  ENTRY_TYPE pd_i = vaddr_start >> 22;
  ENTRY_TYPE pt_i = vaddr_start >> 12 & 0x03FF;
  ENTRY_TYPE *pt;
  int ret = 0;
  printf("pd_i: %d; pt_i: %d\n", pd_i, pt_i);

  while (len > 0) {
    if (!is_pde_present(pd_i))
      return 1; // encountered non-present pde
    pt = (ENTRY_TYPE *)get_addr(page_dir[pd_i]);
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

  dbg_idtr();
  asm volatile("mov eax, %0\n\t"
               "mov cr3, eax\n\t"
               "mov eax, cr0\n\t"
               "or eax, 0x80000001\n\t"
               "mov cr0, eax\n\t" ::"g"(
                   page_dir)); // okay so I have no idea what/why just happened
                               // but I added the g here and now it works?????
  // printf("physaddr of 0x400000 is: %p\n", get_physaddr((void *)0x400000));

  // Test code, I know, really good testing system in this kernel
  // vm_map(0x12345678, 1);
  // vm_map(0x12347678, 1);
  // vm_unmap(0x12345678, 1);
  // dbg_llist();
  // dbg_llist_rev();
  // int map_status = vm_map(0x12345678, 2);
  // printf("vm_map status: %d\n", map_status);
  // printf("physaddr of 0x12345678 is: %p\n", get_physaddr((void
  // *)0x12345678)); printf(" done %p %p\n", &(((ENTRY_TYPE
  // *)page_dir[72])[837]),
  //        ((ENTRY_TYPE *)page_dir[72])[837]);
  // printf("physaddr of 0x12346678 is: %p\n", get_physaddr((void
  // *)0x12346678)); printf("physaddr of 0x12347678 is: %p\n",
  // get_physaddr((void *)0x12347678));
  //
  // vm_unmap(0x12347678, 1);
  // vm_unmap(0x12345678, 2);
  // printf("%c", *((char *)0x12347678));
  // printf("%c", *((char *)0x12346678));
  //
  // printf("\n%d\n", vm_unmap(0x12345678, 2));
  // printf("%c", *((char *)0x12346678));
  log("Paging enabled!\n");
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
    kernel_pt[i] = (i * 0x1000 + 0x8000000) |
                   3; // attributes: supervisor level, read/write, present.
    zero_page_dir[i] = (i * 0x1000) | 3;

    pt_space_pt[i] = (ENTRY_TYPE)(((i * 0x1000 + (ENTRY_TYPE)pt_space)) | 3);
  }
  fill_pde64((pde64_t *)page_dir, (ENTRY_TYPE)zero_page_dir, true, false);
  fill_pde64((pde64_t *)(&page_dir[0x8000000 >> 22]), (unsigned long)kernel_pt,
             true, false);
  fill_pde64((pde64_t *)(&page_dir[1]), (unsigned long)pt_space_pt, true,
             false);

  ENTRY_TYPE *stack_pt = alloc_pt();
  KASSERT(stack_pt);
  for (int i = 0; i < 1024; i++) {
    stack_pt[i] = (i * 0x1000 + MIN_STACK - 0x1000) | 3;
    // log("Stack PTE: %p\n", stack_pt[i]);
  }
  fill_pde64((pde64_t *)(&page_dir[MIN_STACK >> 22]), (ENTRY_TYPE)stack_pt,
             true, false);

  const ENTRY_TYPE VM_HEAP_START = 0x00900000;
  init_physalloc(0x00800000, VM_HEAP_START);
  log("enable_paging: %p\n", enable_paging);
  enable_paging();

  *((uint8_t *)MIN_STACK) = 62;
  log("Test: %d\n", *((uint8_t *)MIN_STACK));

  // KASSERT(vm_map_ext(VM_HEAP_START, 2, NULL, NULL, true, false) == 0);
  KASSERT(vm_map(VM_HEAP_START, 2) == 0);
  init_kalloc((char *)VM_HEAP_START, 8192);

  *((uint8_t *)MIN_STACK) = 63;
  log("Test: %d\n", *((uint8_t *)MIN_STACK));
  // Test code, yes this is a very good way to do testing
  // int *test = kalloc(4);
  // *test = 4;
  // int *test2 = kalloc(4);
  // *test2 = ~4;
  // printf("%p != %p; %d != %d", test, test2, *test, *test2);
  // kfree(test, 4);
  // kfree(test2, 4);
}
