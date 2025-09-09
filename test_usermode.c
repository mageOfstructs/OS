#include <stdint.h>
void test_user_function() {
  const char *test = "Awawawawawawa\n";
  const uint32_t test_sz = 15;
  asm("int 0x80");
  int asdf = 0;
  asdf++;
  asm("push %2\n\t"
      "push %1\n\t"
      "push %0 \n\t"
      "mov eax, 1\n\t"
      "int 0x80" ::"r"(1),
      "m"(test), "r"(test_sz));
  // asm("push eax; int 0x80");
  asm("mov eax, 99; int 0x80");
}
