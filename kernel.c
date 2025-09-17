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

static uint64_t GDT[6];
static uint8_t GDTR[6];

int main() {
  init_serial();
  printf("Main lives at: %p", main);

  // setup GDTR
  GDT[0] = 0;
  for (int i = 1; i < 5; i++) {
    gdte_t *gdte = (gdte_t *)&GDT[i];
    gdte->llimit = 0xffff;
    gdte->lbase = 0;
    gdte->midbase = 0;
    gdte->accessb = 0b10010010;
    if (i % 2 == 1)
      gdte->accessb |= 1 << 3;
    if (i > 2)
      gdte->accessb |= 0b01100000;
    gdte->flags = 0b1100;
    gdte->hlimit = 0b1111;
    gdte->hbase = 0;
  }
  uint32_t gdt_addr = (uint32_t)GDT;
  for (int i = 0; i < 4; i++) {
    GDTR[i + 2] = (gdt_addr >> i * 8) & 0xFF;
  }
  *((uint16_t *)GDTR) = 48; // six GDT entries

  setup_tss((gdte_t *)&GDT[5]);
  printf("%p %d\n", GDTR, *((uint16_t *)GDTR));

  for (int i = 0; i < sizeof(GDT); i++) {
    printf("%p ", ((uint8_t *)GDT)[i]);
  }
  printf("\n");

  // relocate the GDT as setup_vm would otherwise overwrite it with a page
  // table NOTE: This is not a permanent solution! It could be that the
  // kernel grows so big it's code would overlap with the bootsector and
  // overwrite it when it is loaded into memory.
  asm("lgdt %1\n\t"
      "jmp 0x08:next\n\t"
      "next:\n\t"

      // setup segments
      "mov ax, 0x10\n\t"
      "mov ds, ax\n\t"
      "mov ss, ax\n\t"
      "mov es, ax\n\t"
      "mov fs, ax\n\t"
      "mov gs, ax\n\t"

      // load TSS
      "mov ax, 0x28\n\t"
      "ltr ax\n\t" ::"g"(GDT),
      "m"(GDTR));

  printf("Got here!");
  PIC_remap(0x20, 0x28);
  idt_init();
  setup_vm();
  printf("Virtual memory intialized!\n");

  uint16_t buf[256];
  int ret = identify(buf);
  printf("identify: %d\n", ret);
  if (ret == IDENTIFY_ATA) {
    printf("Supports LBA48: %d\n", lba48_support(buf));
    printf("Number of addressable LBA sectors: %p\n", get_lba_cnt(buf));
    init_fs();
    printf("Made it out alive!");

    char file_content[64];
    memset(&file_content, 0, 64);

    fildes_t hello_fd = open_ext2("hello", 0); // perms are not implemented yet
    if (fildes_t_cmp(&hello_fd, &NULL_FD)) {
      printf("open failed!\n");
      return 0;
    }
    KASSERT(read(&hello_fd, 32, file_content) == 32);
    for (int i = 0; i < 6; i++) {
      printf("%p ", file_content[i]);
    }
    printf("\n%s\nend", file_content);
    close_ext2(&hello_fd);

    fildes_t test_fd = open_ext2("test", 0);
    load_new_usermode_prog(&test_fd);
    if (test_fd.type == NULL_TYPE || fildes_t_cmp(&test_fd, &NULL_FD)) {
      printf("open failed!\n");
      return 0;
    }
    KASSERT(read(&test_fd, 6, file_content) == 6);
    printf("\nRet Addr here: %p", file_content);
    printf("File contents:");
    for (int i = 0; i < 64; i++) {
      printf("%p ", (uint8_t)file_content[i]);
    }
  }

  printf("kernel end\n");
}
