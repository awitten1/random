

#include <stdint.h>
#include <stdio.h>
#include <string.h>


uint32_t reverse_bytes(uint32_t b) {
    uint32_t out = 0;
    uint32_t mask = 0xff000000;
    for (int i = 0; i < 4; ++i) {
        out |= (b & (mask >> (i*8)));
    }
    return out;
}

char* vendor_string() {
    static char buf[13];
    uint32_t ebx, ecx, edx;
    asm volatile("xor %%eax, %%eax; cpuid;" : "=b"(ebx), "=c"(ecx), "=d"(edx) :: "eax");
    ebx = reverse_bytes(ebx);
    ecx = reverse_bytes(ecx);
    edx = reverse_bytes(edx);
    memcpy(buf, &ebx, 4);
    memcpy(buf + 4, &edx, 4);
    memcpy(buf + 8, &ecx, 4);
    buf[12] = '\0';
    return buf;
}

uint32_t family() {
    uint32_t ebx, ecx, edx, eax;
    asm volatile("mov $0x01, %%eax; cpuid;" : "=a"(eax) :: "ebx", "ecx", "edx");
    return (((eax >> 20) & 0xff)) + ((eax >> 8) & 0xf);
}


uint32_t model() {
    uint32_t ebx, ecx, edx, eax;
    asm volatile("mov $0x01, %%eax; cpuid;" : "=a"(eax) :: "ebx", "ecx", "edx");
    return (((eax >> 16) & 0xf) << 4) | ((eax >> 4) & 0xf);
}


int main() {
    fprintf(stdout, "vendor string: %s, family: %d, model: %d\n", vendor_string(), family(), model());
    return 0;
}