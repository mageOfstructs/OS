#include "ata.h"
#include "fildes.h"
#include "fs/ext2.h"
#include "idt.h"
#include "mem.h"
#include "pic.h"
#include "printf.h"
#include "serial.h"
#include "usermode.h"
#include "utils.h"
#include "vm.h"
#include <stdint.h>

STRUCT_EQ(fildes_t);

static uint64_t GDT[6];
static uint8_t GDTR[6];

int main() {
  init_serial();
  // setup GDTR
  uint32_t gdt_addr = (uint32_t)GDT;
  for (int i = 0; i < 4; i++) {
    GDTR[i + 2] = (gdt_addr >> i * 8) & 0xFF;
  }
  *((uint16_t *)GDTR) = 48; // six GDT entries
  printf("%p %d\n", GDTR, *((uint16_t *)GDTR));

  setup_tss((gdte_t *)&GDT[5]);
  printf("%p %d\n", GDTR, *((uint16_t *)GDTR));

  // relocate the GDT as setup_vm would otherwise overwrite it with a page table
  // NOTE: This is not a permanent solution! It could be that the kernel grows
  // so big it's code would overlap with the bootsector and overwrite it when it
  // is loaded into memory.
  asm("mov ecx, 8*5\n\t"
      "mov esi, 0x7c42\n\t"
      "mov edi, %0\n\t"
      "rep; movsb\n\t"
      // "mov ecx, 2\n\t"
      // "mov esi, 0x7c5a\n\t"
      // "mov edi, %1\n\t"
      // "rep; movsb\n\t"
      "lgdt %1\n\t"
      "mov ax, 0x28\n\t" // load TSS
      "ltr ax\n\t" ::"g"(GDT),
      "m"(GDTR));

  PIC_remap(0x20, 0x28);
  idt_init();
  setup_vm(); // TODO: need to setup an address space for usermode

  // jump_usermode();

  uint16_t buf[256];
  int ret = identify(buf);
  printf("identify: %d\n", ret);
  if (ret == IDENTIFY_ATA) {
    printf("Supports LBA48: %d\n", lba48_support(buf));
    printf("Number of addressable LBA sectors: %p\n", get_lba_cnt(buf));
    init_fs();
    printf("Made it out alive!");

    fildes_t hello_fd = open_ext2("hello", 0); // perms are not implemented yet
    if (fildes_t_cmp(&hello_fd, &NULL_FD)) {
      printf("open failed!\n");
      return 0;
    }
    char file_content[64];
    memset(&file_content, 0, 64);
    KASSERT(read(&hello_fd, 32, file_content) == 32);
    printf("%s\n", file_content);
    close_ext2(&hello_fd);

    fildes_t test_fd = open_ext2("test", 0);
    if (fildes_t_cmp(&hello_fd, &NULL_FD)) {
      printf("open failed!\n");
      return 0;
    }
    KASSERT(read(&test_fd, 6, file_content) == 6);
    printf("File contents:");
    for (int i = 0; i < 6; i++) {
      printf("%p ", file_content[i]);
    }
  }

  // load_usermode_prog(&test_fd);

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
