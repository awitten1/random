
#include <stdlib.h>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  long bytes_per_second = std::stol(argv[1]);
  long total_bytes = 5000000000;
  if (argc == 3) {
    total_bytes = std::stol(argv[2]);
  }
  long allocated_bytes = 0;
  long alloc_size = bytes_per_second / 10;
  for (;;) {
    (void)malloc(alloc_size);
    allocated_bytes += alloc_size;
    if (alloc_size > total_bytes) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}

