#include "usermode.h"
#include "printf.h"
#include <stdint.h>

static tss_t TSS;

void setup_tss(gdte_t *gdt) {
  uint32_t tss_addr = (uint32_t)&TSS;
  gdt->lbase = (uint16_t)tss_addr;
  gdt->midbase = (uint8_t)(tss_addr >> 16);
  gdt->hbase = (uint8_t)(tss_addr >> 24);
  // printf("%d\n", *((uint16_t *)0x8170));

  uint32_t tss_sz = sizeof(TSS);
  gdt->llimit = (uint16_t)tss_sz;
  gdt->hlimit = (uint8_t)(tss_sz >> 16);

  gdt->accessb = 0x89;
  gdt->flags = 0x4;

  TSS.ss0 = 0x10;
  TSS.esp0 = 0x90000;
  TSS.iomap_base = sizeof(TSS);
}
