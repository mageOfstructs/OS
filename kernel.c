#include "idt.h"
#include "printf.h"
#include <stdint.h>
#include "pic.h"

int main() {
  idt_init();
  PIC_remap(0x20, 0x28);

  // enable_cursor(0, 15);
  // *((int *)0xb8000) = 0x07690748;
  char *test = "Test";
  int asdf = 43;
  // printf("%s %s %s", test, "asdf", "fdas");
  printf("test\n");
  printf("test2\n");
  uint32_t eip;
  asm("mov %0, $" : "=r"(eip));
  printf("cur eip: %p\n", eip);
  // printf("test3\n");
  asm("int 0x00");
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
