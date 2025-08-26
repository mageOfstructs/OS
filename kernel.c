#include "idt.h"
#include "printf.h"

int main() {
  idt_init();
  // enable_cursor(0, 15);
  // *((int *)0xb8000) = 0x07690748;
  char *test = "Test";
  int asdf = 43;
  // printf("%s %s %s", test, "asdf", "fdas");
  printf("test\n");
  printf("test2\n");
  printf("test3\n");
  asm("int 0x80");
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
