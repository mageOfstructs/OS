#include "ata.h"
#include "idt.h"
#include "pic.h"
#include "printf.h"
#include "vm.h"
#include <stdint.h>

static uint64_t GDT[3];
static uint8_t GDTR[6];

int main() {
  // relocate the GDT as setup_vm would otherwise overwrite it with a page table
  // NOTE: This is not a permanent solution! It could be that the kernel grows
  // so big it's code would overlap with the bootsector and overwrite it when it
  // is loaded into memory.
  uint32_t gdt_addr = (uint32_t)GDT;
  for (int i = 0; i < 4; i++) {
    GDTR[i + 2] = (gdt_addr >> i * 8) & 0xFF;
  }
  asm("mov ecx, 8*3\n\t"
      "mov esi, 0x7c42\n\t"
      "mov edi, %0\n\t"
      "rep; movsb\n\t"
      "mov ecx, 2\n\t"
      "mov esi, 0x7c5a\n\t"
      "mov edi, %1\n\t"
      "rep; movsb\n\t"
      "lgdt %2" ::"g"(GDT),
      "g"(GDTR), "m"(GDTR));
  PIC_remap(0x20, 0x28);
  idt_init();
  setup_vm();

  uint8_t buf[256];
  // int ret = identify(buf);
  int ret = 1;
  printf("identify: %d", ret);
  if (ret == IDENTIFY_ATA) {
    for (int i = 0; i < 256; i++) {
      printf("%p", buf[i]);
    }
  }

  // enable_cursor(0, 15);
  // *((int *)0xb8000) = 0x07690748;
  char *test = "Test";
  int asdf = 43;
  // printf("%c", *((char *)0x99999999));
  // printf("%s %s %s", test, "asdf", "fdas");
  printf("test\n");
  // printf("test2\n");
  // int tmp = 1 / 0;
  // printf("test3\n");
  // asm volatile("int 0x00");
  // printf("test2");
  // printf("%s %d", test, asdf);
  // printf("%p", &asdf);
  // write_str("Tessssssssssssssssssssssssssssssssssssssssssssssst",
  //           (volatile u16 *)0xB8000);
  // char *mem = (char *)0xb8000;
  // for (int i = 0; i < 100; i++) {
  //   *mem = 'Q';
  //   mem += 2;
  // }
}
