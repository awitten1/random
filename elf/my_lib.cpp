

#include <string>
#include <vector>
#include "my_lib.h"

// gcc -c my_lib.cpp

int my_uninitialized_global;
int my_initialized_global = 1;

int f(char x) {
  return x + 2;
}

void my_func_str(std::string& x) {
}

static void my_static_func_str(std::string& x) {
}

void MyClass::f() {}

int MyClass::g(std::vector<int> y) {
  return 0;
}
