#include "asm_abi.h"

// int64_t sum_i64_unroll4_asm(const int64_t *a /*x0*/, size_t n /*x1*/)
//
// Same result as 02, but four elements per iteration using ldp (load pair,
// two 64-bit registers from one instruction) and two independent accumulator
// chains so the adds can dual-issue instead of serialising on one register.
FUNC SYM(sum_i64_unroll4_asm)
    mov     x2, #0              // acc0
    mov     x3, #0              // acc1
    lsr     x4, x1, #2          // x4 = n / 4  (whole blocks)
    cbz     x4, Lu4_tail
Lu4_loop:
    ldp     x5, x6, [x0], #16   // a[0], a[1]
    ldp     x7, x8, [x0], #16   // a[2], a[3]
    add     x2, x2, x5
    add     x3, x3, x6
    add     x2, x2, x7
    add     x3, x3, x8
    subs    x4, x4, #1
    b.ne    Lu4_loop
Lu4_tail:
    add     x2, x2, x3          // fold the two chains together
    ands    x4, x1, #3          // x4 = n % 4, and set flags
    b.eq    Lu4_done
Lu4_tail_loop:
    ldr     x5, [x0], #8
    add     x2, x2, x5
    subs    x4, x4, #1
    b.ne    Lu4_tail_loop
Lu4_done:
    mov     x0, x2
    ret
