// Small compatibility shims so the .S files read the same everywhere.
// These files are .S (capital S), so the C preprocessor runs on them first.
#ifndef LEARN_ARM_ASM_ABI_H
#define LEARN_ARM_ASM_ABI_H

#if defined(__APPLE__)
//  Mach-O prefixes every C symbol with an underscore.
#  define SYM(name) _##name
#else
#  define SYM(name) name
#endif

// Declare a global, 4-byte-aligned function `name`.
.macro FUNC name
    .text
    .p2align 2
    .globl \name
\name:
.endm

#endif
