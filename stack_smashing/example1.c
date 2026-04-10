void function1(int a, int b, int c) {
   char buffer1[5];
   char buffer2[10];
   buffer1[0] = 8;
   buffer1[4] = 9;
   buffer2[0] = 3;
   buffer2[9] = 11;
}

void function2() {
  function1(1,2,3);
}