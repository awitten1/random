

#include <string>
#include <vector>
#include "my_lib.h"

// g++ -c my_lib.cpp

int my_uninitialized_global;
int my_initialized_global = 1;
static int static_my_uninitialized_global;
static int static_my_initialized_global = 1;

const char* string_literal = "my string literal";

int f(char x) {
  return x + 2;
}


// The static version and the version in the anon ns have local binding (STB_LOCAL)
// whereas the non-static one has global binding (STB_GLOBAL).
void my_func_str(std::string& x) {
}
static void my_static_func_str(std::string& x) {
}
namespace {
void my_anon_ns_func_str(std::string& x) {
}
}

void MyClass::f() {}

int MyClass::g(std::vector<int> y) {
  return 0;
}
