void test_user_function() {
  asm("int 0x80");
  for (;;)
    ;
}
