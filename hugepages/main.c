
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>

#define SMALL_PAGE_SIZE (1 << 12)
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define HUGE_PAGE_SIZE (1 << 21)

void write_into_random_pages(int num_pages, char* buf, size_t page_size) {
  buf[page_size*(rand() % num_pages)] = 'b';
}


int main(int argc, char** argv) {
  int num_pages = 1024;
  int options = MAP_ANONYMOUS | MAP_PRIVATE;
  int sz = num_pages * SMALL_PAGE_SIZE;
  int page_size = SMALL_PAGE_SIZE;
  /*
  if (argc > 1 && strcmp(argv[1], "t") == 0) {
    options |= MAP_HUGETLB;
    options |= MAP_HUGE_2MB;
    num_pages = 2;
    sz = num_pages * HUGE_PAGE_SIZE;
    page_size = HUGE_PAGE_SIZE;
  }
  */

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
  for (int i = 0; i < num_pages; ++i) {
    buf[i * page_size] = 'a';
  }

  sleep(30);

  write_into_random_pages(num_pages, buf, page_size);

  return 0;

}


