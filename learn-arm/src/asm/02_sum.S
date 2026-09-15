#include "asm_abi.h"

// int64_t sum_i64_asm(const int64_t *a /*x0*/, size_t n /*x1*/)
//
// The most literal loop possible: one element per iteration.
FUNC SYM(sum_i64_asm)
    mov     x2, #0              // x2 = accumulator
    cbz     x1, Lsum_done       // compare-and-branch-if-zero: nothing to do
Lsum_loop:
    ldr     x3, [x0], #8        // post-index: load *x0, THEN x0 += 8
    add     x2, x2, x3
    subs    x1, x1, #1          // the 's' suffix makes it set the flags
    b.ne    Lsum_loop
Lsum_done:
    mov     x0, x2
    ret
