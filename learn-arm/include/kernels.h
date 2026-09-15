// Declarations for every kernel, in two flavours:
//   *_c   -> plain C reference implementation (src/c_kernels.c)
//   *_asm -> hand-written assembly       (src/asm/*.S)
// The tests check that the two agree; the benchmark races them.
#ifndef LEARN_ARM_KERNELS_H
#define LEARN_ARM_KERNELS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 01 - the "hello world": two arguments in, one out. */
int64_t add_i64_c(int64_t a, int64_t b);
int64_t add_i64_asm(int64_t a, int64_t b);

/* 02 - a counted loop over memory. */
int64_t sum_i64_c(const int64_t *a, size_t n);
int64_t sum_i64_asm(const int64_t *a, size_t n);

/* 03 - same loop, unrolled 4x with ldp (load-pair). */
int64_t sum_i64_unroll4_asm(const int64_t *a, size_t n);

/* 04 - same loop with NEON: 4 lanes per iteration. */
int64_t sum_i64_neon_asm(const int64_t *a, size_t n);

/* 05 - float dot product, NEON fmla (fused multiply-add). */
float dot_f32_c(const float *a, const float *b, size_t n);
float dot_f32_asm(const float *a, const float *b, size_t n);

#ifdef __cplusplus
}
#endif
#endif /* LEARN_ARM_KERNELS_H */
