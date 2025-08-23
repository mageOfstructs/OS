#include "printf.h"

int main() {
  // *((int *)0xb8000) = 0x07690748;
  char *test = "Test";
  write_str("Tessssssssssssssssssssssssssssssssssssssssssssssst",
            (volatile u16 *)0xB8000);
  // char *mem = (char *)0xb8000;
  // for (int i = 0; i < 100; i++) {
  //   *mem = 'Q';
  //   mem += 2;
  // }
}
