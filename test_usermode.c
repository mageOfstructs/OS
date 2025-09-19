#include "stdlib/syscalls.h"
#include "printf.h"
#include <stdint.h>

void test_user_function() {
  const char *test = "Awawawawawawa\n";
  const char *path = "test2";
  const uint32_t test_sz = 14; // we don't need the null byte
  char buf[4];
  buf[3] = '\n';
  asm("int 0x80");
  int asdf = 0;
  asdf++;

  write(1, test, test_sz);
  read(0, buf, 3);
  write(1, buf, 4);
  printf("Hello World from Usermode printf Here are some numbers to prove this "
         "works: %d %p!\n",
         32, 0x1000);
  uint32_t pid = fork();
  printf("fork ret: %d\n", pid);
  if (pid != 0)
    printf("Hello World from Parent!\n");
  else
    printf("Hello World from the Child!\n");
  exit(0);
  // asm("push %2\n\t"
  //     "push %1\n\t"
  //     "push %0 \n\t"
  //     "mov eax, 1\n\t"
  //     "int 0x80\n\t" ::"r"(1),
  //     "m"(test), "r"(test_sz));
  // asm("push %2\n\t"
  //     "push %1\n\t"
  //     "push %0 \n\t"
  //     "mov eax, 2\n\t"
  //     "int 0x80" ::"r"(0),
  //     "r"(&buf), "r"(3));
  // asm("push %2\n\t"
  //     "push %1\n\t"
  //     "push %0 \n\t"
  //     "mov eax, 1\n\t"
  //     "int 0x80\n\t" ::"r"(1),
  //     "r"(&buf), "r"(4));
  // asm("mov eax, 12\n\t"
  //     "int 0x80" ::
  //         : "eax");
  // asm("push %2\n\t"
  //     "push %1\n\t"
  //     "push %0 \n\t"
  //     "mov eax, 1\n\t"
  //     "int 0x80\n\t" ::"r"(1),
  //     "m"(test), "r"(test_sz)
  //     : "eax");
  // asm("mov eax, 100\n\t"
  //     "int 0x80" ::
  //         : "eax");
  // sys_exec:
  // asm("push %0\n\t"
  //     "mov eax, 11\n\t"
  //     "int 0x80" ::"m"(path));
  // sys_hlt:
  // asm("mov eax, 99; int 0x80");
}
