#include "asm_abi.h"

// int64_t sum_i64_neon_asm(const int64_t *a /*x0*/, size_t n /*x1*/)
//
// NEON register naming, all views of the same 32 physical registers:
//   q0 = 128 bits   d0 = low 64   s0 = low 32   h0 = low 16   b0 = low 8
//   v0.2d = two 64-bit lanes, v0.4s = four 32-bit lanes, v0.16b = sixteen bytes
// `add v0.2d, v0.2d, v1.2d` adds both lanes at once; there is no flag-setting
// or carry between lanes.
FUNC SYM(sum_i64_neon_asm)
    movi    v0.2d, #0           // acc0 = {0, 0}
    movi    v1.2d, #0           // acc1 = {0, 0}
    lsr     x4, x1, #2          // 4 int64s per iteration
    cbz     x4, Lneon_tail
Lneon_loop:
    ldp     q2, q3, [x0], #32   // 32 bytes in one instruction
    add     v0.2d, v0.2d, v2.2d
    add     v1.2d, v1.2d, v3.2d
    subs    x4, x4, #1
    b.ne    Lneon_loop
Lneon_tail:
    add     v0.2d, v0.2d, v1.2d
    addp    d0, v0.2d           // horizontal add: d0 = lane0 + lane1
    fmov    x2, d0              // move the vector register into a GPR
    ands    x4, x1, #3
    b.eq    Lneon_done
Lneon_tail_loop:
    ldr     x5, [x0], #8
    add     x2, x2, x5
    subs    x4, x4, #1
    b.ne    Lneon_tail_loop
Lneon_done:
    mov     x0, x2
    ret
