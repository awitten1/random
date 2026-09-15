// Plain C reference implementations. These are what the assembly has to match
// for correctness, and what it has to beat (or at least explain) for speed.
#include "kernels.h"

int64_t add_i64_c(int64_t a, int64_t b) { return a + b; }

int64_t sum_i64_c(const int64_t *a, size_t n) {
    int64_t acc = 0;
    for (size_t i = 0; i < n; ++i) acc += a[i];
    return acc;
}

float dot_f32_c(const float *a, const float *b, size_t n) {
    float acc = 0.0f;
    for (size_t i = 0; i < n; ++i) acc += a[i] * b[i];
    return acc;
}
