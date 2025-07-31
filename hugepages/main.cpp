
#include <cstdlib>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include "perfcxx/perf-lib.hpp"

#define SMALL_PAGE_SIZE (1 << 12)
#define MAP_HUGE_2MB (30 << MAP_HUGE_SHIFT)
#define HUGE_PAGE_SIZE (1 << 21)

void write_into_random_pages(int sz, char* buf, int iters) {
  int num_pages = sz / SMALL_PAGE_SIZE;
  PerfEventGroup events(PERF_COUNT_HW_CPU_CYCLES, PERF_TYPE_HARDWARE, "cycles");
  events.AddEvent(0xff45, PERF_TYPE_RAW, "dTLB_misses");
  events.Enable();

  for (int i = 0; i < iters; ++i)
    buf[SMALL_PAGE_SIZE*(rand() % num_pages)] = 'b';

  events.Disable();
  auto e = events.ReadEvents();
  for (auto& x : e) {
    std::cout << x.first << ", " << *x.second << std::endl;
  }
}


int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "Usage: ./thp <t or f to indicate thp> <iters> <buf shift>\n");
    exit(EXIT_FAILURE);
  }
  int sz = (1 << std::stoi(argv[3]));
  int options = MAP_ANONYMOUS | MAP_PRIVATE;
  int num_pages = sz / SMALL_PAGE_SIZE;
  int page_size = SMALL_PAGE_SIZE;

  char* buf = (char*)mmap(NULL, sz, PROT_READ | PROT_WRITE, options, -1, 0);
  if (buf == MAP_FAILED) {
    printf("enomem? %d\n", errno == ENOMEM);
    err(EXIT_FAILURE, "mmap failed");
  }
  printf("buf address = %p\n", buf);

  if (argc > 1 && strcmp(argv[1], "t") == 0) {
    printf("setting hugepages\n");
    int ret = madvise((void*)buf, sz, MADV_HUGEPAGE);
    assert(ret == 0);
  }
  for (int i = 0; i < sz; i += SMALL_PAGE_SIZE) {
    buf[i] = 'a';
  }

  //sleep(30);

  int iters = std::stoi(argv[2]);
  write_into_random_pages(sz, buf, iters);

  return 0;

}


