#include <stdio.h>

void function(int a, int b, int c) {
   char buffer1[5];
   char buffer2[10];
   *buffer1 = 56;
   buffer2[9] = 34;
   long *ret = (long*)(buffer1 + 21);
   (*ret) += 7;
}

int main() {
  int x;

  x = 11;
  function(1,2,3);
  x = 1;
  printf("%d\n",x);
  return 0;
}