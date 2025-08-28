#include "idt.h"
#include "vm.h"
#include "pic.h"
#include "printf.h"
#include <stdint.h>

int main() {
  idt_init();
  PIC_remap(0x20, 0x28);
  setup_vm();

  // enable_cursor(0, 15);
  // *((int *)0xb8000) = 0x07690748;
  char *test = "Test";
  uint32_t eip;
  asm("mov %0, $" : "=r"(eip));
  printf("cur eip: %p\n", eip);
  int asdf = 43;
  // printf("%s %s %s", test, "asdf", "fdas");
  printf("test\n");
  printf("test2\n");
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
