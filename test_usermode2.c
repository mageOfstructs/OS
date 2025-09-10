#include <stdint.h>
void test_user_function() {
  const char *test = "uwuwuwuwuwuwuwuwwuuwwu\n";
  const uint32_t test_sz = 23; // we don't need the null byte
  asm("push %2\n\t"
      "push %1\n\t"
      "push %0 \n\t"
      "mov eax, 1\n\t"
      "int 0x80\n\t" ::"r"(1),
      "m"(test), "r"(test_sz));
  asm("mov eax, 99; int 0x80");
}
