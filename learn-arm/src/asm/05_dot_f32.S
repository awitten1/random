#include "asm_abi.h"

// float dot_f32_asm(const float *a /*x0*/, const float *b /*x1*/, size_t n /*x2*/)
//
// Floating-point arguments and the return value live in v0..v7 (s0 here for a
// single float), NOT in x0..x7 - so the two pointers and the count still land
// in x0, x1, x2.
//
// fmla is a *fused* multiply-add: acc += a * b with a single rounding. That
// makes the result differ in the last bit from a separate fmul + fadd, which
// is exactly why the test compares with a tolerance instead of ==.
FUNC SYM(dot_f32_asm)
    movi    v0.4s, #0           // acc0: four float lanes
    movi    v1.4s, #0           // acc1
    lsr     x3, x2, #3          // 8 floats per iteration
    cbz     x3, Ldot_tail
Ldot_loop:
    ldp     q2, q3, [x0], #32   // a[0..7]
    ldp     q4, q5, [x1], #32   // b[0..7]
    fmla    v0.4s, v2.4s, v4.4s
    fmla    v1.4s, v3.4s, v5.4s
    subs    x3, x3, #1
    b.ne    Ldot_loop
Ldot_tail:
    fadd    v0.4s, v0.4s, v1.4s
    faddp   v0.4s, v0.4s, v0.4s // pairwise: lanes {0+1, 2+3, 0+1, 2+3}
    faddp   s0, v0.2s           // s0 = lane0 + lane1 -> the scalar total
    ands    x3, x2, #7
    b.eq    Ldot_done
Ldot_tail_loop:
    ldr     s2, [x0], #4
    ldr     s3, [x1], #4
    fmadd   s0, s2, s3, s0      // s0 = s2 * s3 + s0
    subs    x3, x3, #1
    b.ne    Ldot_tail_loop
Ldot_done:
    ret                         // the return value is already in s0
