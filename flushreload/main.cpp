

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// This code is from
// https://www.usenix.org/system/files/conference/usenixsecurity14/sec14-paper-yarom.pdf
int probe(char *adrs) {
    volatile unsigned long time;

    asm volatile (
    " mfence \n"
    " lfence \n"
    " rdtsc \n"
    " lfence \n"
    " movl %%eax, %%esi \n"
    " movl (%1), %%eax \n"
    " lfence \n"
    " rdtsc \n"
    " subl %%esi, %%eax \n"
    " clflush 0(%1) \n"
    : "=a" (time)
    : "c" (adrs)
    : "%esi", "%edx");

    return time;
}

const long page_size = sysconf(_SC_PAGE_SIZE);

uint16_t get_cache_line_size() {
    const char* file_path = "/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size";
    std::ifstream f(file_path);

    std::string line;
    std::getline(f, line);
    return std::stoi(line);
}

int main(int argc, char** argv) {
    std::string f = argv[1];
    int fd = open(f.c_str(), O_RDONLY);
    struct stat st;
    ::fstat(fd, &st);
    void * buf = ::mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    // Numbers are off when stepping by cache line size.  That's because of the CPU prefetcher.
    // But the prefetcher won't prefetch across page boundaries.
#ifdef CACHE_LINE
    uint32_t step_size = get_cache_line_size();
#else
    uint32_t step_size = page_size;
#endif
    long num_cache_lines = (st.st_size + step_size - 1) / step_size;
    for (int idx = 0; idx < num_cache_lines; ++idx) {
        auto probe_cycles = probe((char*)buf + step_size*idx);
        std::cout << idx << "," << probe_cycles << std::endl;
    }

    munmap(buf, st.st_size);
    close(fd);
    return 0;
}
