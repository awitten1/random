// Correctness harness. Every kernel is checked against its C reference over a
// spread of lengths, including the awkward ones (0, and sizes that are not a
// multiple of the unroll factor) where tail handling usually breaks.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "kernels.h"

static int failures = 0;

static void check_i64(const char *what, size_t n, int64_t got, int64_t want) {
    if (got != want) {
        printf("FAIL %-24s n=%-6zu got=%lld want=%lld\n", what, n,
               (long long)got, (long long)want);
        ++failures;
    }
}

static void check_f32(const char *what, size_t n, float got, float want) {
    // fmla rounds once where the C loop rounds twice, so compare relatively.
    float scale = fabsf(want) > 1.0f ? fabsf(want) : 1.0f;
    if (fabsf(got - want) > 1e-4f * scale) {
        printf("FAIL %-24s n=%-6zu got=%.9g want=%.9g\n", what, n, (double)got,
               (double)want);
        ++failures;
    }
}

int main(void) {
    static const size_t sizes[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 1000, 4097};
    const size_t max_n = 4097;

    int64_t *xi = malloc(max_n * sizeof *xi);
    float *xf = malloc(max_n * sizeof *xf);
    float *yf = malloc(max_n * sizeof *yf);
    srand(1234);
    for (size_t i = 0; i < max_n; ++i) {
        xi[i] = (int64_t)rand() - RAND_MAX / 2;
        xf[i] = (float)rand() / (float)RAND_MAX;
        yf[i] = (float)rand() / (float)RAND_MAX;
    }

    check_i64("add_i64_asm", 2, add_i64_asm(40, 2), add_i64_c(40, 2));
    check_i64("add_i64_asm neg", 2, add_i64_asm(-7, 3), add_i64_c(-7, 3));

    for (size_t k = 0; k < sizeof sizes / sizeof *sizes; ++k) {
        size_t n = sizes[k];
        int64_t want = sum_i64_c(xi, n);
        check_i64("sum_i64_asm", n, sum_i64_asm(xi, n), want);
        check_i64("sum_i64_unroll4_asm", n, sum_i64_unroll4_asm(xi, n), want);
        check_i64("sum_i64_neon_asm", n, sum_i64_neon_asm(xi, n), want);
        check_f32("dot_f32_asm", n, dot_f32_asm(xf, yf, n), dot_f32_c(xf, yf, n));
    }

    free(xi); free(xf); free(yf);
    if (failures == 0) printf("all kernels match their C reference\n");
    return failures == 0 ? 0 : 1;
}
