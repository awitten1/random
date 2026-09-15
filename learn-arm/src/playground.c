// Scratch pad: call a kernel, print what came back. Build and run with
//   cmake --build build --target playground && ./build/playground
#include <stdio.h>
#include "kernels.h"

int main(void) {
    printf("add_i64_asm(40, 2)      = %lld\n", (long long)add_i64_asm(40, 2));

    int64_t v[10];
    for (int i = 0; i < 10; ++i) v[i] = i + 1;
    printf("sum_i64_asm(1..10)      = %lld\n", (long long)sum_i64_asm(v, 10));
    printf("sum_i64_unroll4_asm     = %lld\n", (long long)sum_i64_unroll4_asm(v, 10));
    printf("sum_i64_neon_asm        = %lld\n", (long long)sum_i64_neon_asm(v, 10));

    float a[5] = {1, 2, 3, 4, 5}, b[5] = {5, 4, 3, 2, 1};
    printf("dot_f32_asm             = %.1f\n", (double)dot_f32_asm(a, b, 5));
    return 0;
}
