# learn-arm

A small CMake playground for writing AArch64 assembly by hand, calling it from
C, and measuring it against a C reference with Google Benchmark.

Target: Apple silicon / macOS (Mach-O, arm64). The `.S` files use `SYM()` for
the leading-underscore symbol prefix, so they are close to portable to Linux
aarch64 too.

## Layout

```
include/kernels.h     one declaration per kernel, _c and _asm flavours
src/asm/*.S           the hand-written assembly (this is the part you edit)
src/c_kernels.c       C reference implementations, built -O3
src/playground.c      scratch main() for poking at a kernel
tests/test_kernels.c  asm vs. C across awkward sizes; run via ctest
bench/bench_kernels.cc Google Benchmark harness
```

## Build and run

```sh
cmake -S . -B build
cmake --build build -j

ctest --test-dir build --output-on-failure   # correctness first
./build/playground                           # quick manual poke
./build/bench_kernels                        # all benchmarks
./build/bench_kernels --benchmark_filter=Dot # just one family
```

Two targets exist purely as learning aids:

```sh
cmake --build build --target disasm    # llvm-objdump of your assembled .S
cmake --build build --target cref-asm  # what clang -O3 emits for the C version
```

`disasm` is the honest feedback loop: it shows what the assembler actually
encoded, including any pseudo-instruction it expanded on your behalf.

## Adding a kernel

1. Declare `foo_c` and `foo_asm` in `include/kernels.h`.
2. Write the C reference in `src/c_kernels.c`.
3. Write `src/asm/06_foo.S`, starting with `#include "asm_abi.h"` and
   `FUNC SYM(foo_asm)`. The glob picks the file up on the next build — no
   CMake edit needed.
4. Add a case to `tests/test_kernels.c` and a `BENCHMARK_CAPTURE` in
   `bench/bench_kernels.cc`.

## How the assembly gets built

`project(... LANGUAGES C CXX ASM)` is what teaches CMake to assemble `.S`
files; it drives them through the C compiler, which hands them to clang's
integrated assembler. The capital `.S` matters: it runs the C preprocessor
first, which is why `#include "asm_abi.h"` works. A lowercase `.s` would be
assembled verbatim.

`asm_kernels` is a plain static library, so the C and C++ sides just link
against it and call the symbols as ordinary `extern "C"` functions.

## The kernels, in order

| file | idea |
|---|---|
| `01_add.S` | the ABI: arguments in `x0..x7`, return in `x0`, `ret` jumps to `x30` |
| `02_sum.S` | a counted loop, post-indexed `ldr`, `subs`/`b.ne` on the flags |
| `03_sum_unroll.S` | `ldp` load-pair, 4x unroll, two accumulators to break the dependency chain |
| `04_sum_neon.S` | NEON `v` registers, `add v0.2d`, horizontal reduction with `addp` |
| `05_dot_f32.S` | floats return in `s0`/`v0`, `fmla` fused multiply-add, `faddp` reduction |

## What the numbers actually show

Worth knowing before you get discouraged by your first benchmark run:

- On the **integer sum**, the C version beats all three assembly versions.
  Integer addition is associative, so clang is free to vectorise and unroll
  the reduction far more aggressively than the hand-written loops here.
  `cmake --build build --target cref-asm` shows exactly what it did.
- On the **float dot product**, the assembly wins by roughly 6x. Floating-point
  addition is *not* associative, so without `-ffast-math` the compiler must
  keep the C loop strictly sequential — one `fmadd` at a time — while the
  assembly is free to use eight independent lanes.

That contrast is the real lesson: hand-written assembly pays off where the
compiler is forbidden from making a transformation, not where it merely
hasn't. Both effects are also size-dependent — the benchmarks sweep from 64
elements (L1-resident) to 4M (memory-bound), and past L2 everything converges
toward the same bandwidth ceiling.

## Debugging assembly

The `.S` files are built with `-g`, so lldb can step through the source:

```sh
lldb ./build/playground
(lldb) b sum_i64_neon_asm
(lldb) run
(lldb) di -f          # disassemble the current frame
(lldb) register read  # x0..x30
(lldb) register read --format int64 v0   # NEON lanes
(lldb) si             # step one instruction
```
