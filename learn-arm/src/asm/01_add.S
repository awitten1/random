#include "asm_abi.h"

// int64_t add_i64_asm(int64_t a, int64_t b)
//
// AAPCS64 in one breath:
//   x0..x7   integer/pointer arguments, x0 (and x1) also hold the return value
//   x8       indirect result pointer
//   x9..x15  caller-saved scratch: use freely
//   x19..x28 callee-saved: save them before use, restore before ret
//   x29 fp, x30 lr, sp stack pointer (must stay 16-byte aligned)
//   wN       is the low 32 bits of xN; writing wN zeroes the top 32 bits
//
// A leaf function that touches nothing but x0..x15 needs no prologue at all.
FUNC SYM(add_i64_asm)
    add     x0, x0, x1          // x0 = a + b, and x0 is also the return register
    ret                         // branch to x30 (the link register)
