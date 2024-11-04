#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#define PAGE_SIZE (1ULL << 12) // 4KiB
#define NUM_PAGES (1ULL << 19) // ~500K blocks. 4KiB * 500K = 2GiB

int main() {

  // Reserve contiguous block of virtual memory.
  // cat /proc/<pid>/status | grep VmSize/VmPeak
  char* buf = mmap(NULL, PAGE_SIZE * NUM_PAGES,
    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (buf == MAP_FAILED) {
    printf("mmap failed\n");
    abort();
  }

  // Allocate physical memory. VmRSS now goes up.
  // cat /proc/<pid>/status | grep -i vm
  for (int j = 0; j < NUM_PAGES; j++) {
    // pick a random page.
    int page_id = rand() % NUM_PAGES;
    int offset = page_id * PAGE_SIZE;
    buf[offset]++;
  }

  printf("done mmap 1\n");
  sleep(30);
  printf("done mmap 2\n");

  // With either an anonymous or file backed mapping, madvise with MADV_DONTNEED
  // drops the physical memory associated with this range.
  // Therefore VmRSS drops here.
  for (int j = 0; j < NUM_PAGES / 2; ++j) {
    int offset = j * PAGE_SIZE;
    int ret = madvise(buf + offset, PAGE_SIZE, MADV_DONTNEED);
    if (ret == -1) {
      printf("failed madvise %d\n", errno);
      abort();
    }
  }
  printf("madv dontneed\n");

  pause();
  return 0;
}

