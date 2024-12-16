

#include <stdio.h>

void g() {
  printf("a");
}

void f() {
  printf("a");
  g();
}

int main() {
  printf("hello\n");
  f();
  return 0;
}
