// Google Benchmark harness. Run everything:   ./build/bench_kernels
// One family:   ./build/bench_kernels --benchmark_filter=sum
// Stable-ish numbers: --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
#include <benchmark/benchmark.h>

#include <cstdint>
#include <random>
#include <vector>

extern "C" {
#include "kernels.h"
}

namespace {

constexpr int64_t kMinN = 1 << 6;   // fits in L1
constexpr int64_t kMaxN = 1 << 22;  // well past L2: now memory-bound

std::vector<int64_t> MakeI64(size_t n) {
    std::mt19937_64 rng(42);
    std::vector<int64_t> v(n);
    for (auto &x : v) x = static_cast<int64_t>(rng() & 0xffff);
    return v;
}

std::vector<float> MakeF32(size_t n, uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::vector<float> v(n);
    for (auto &x : v) x = dist(rng);
    return v;
}

template <typename Fn>
void BM_Sum(benchmark::State &state, Fn fn) {
    const size_t n = static_cast<size_t>(state.range(0));
    const auto data = MakeI64(n);
    for (auto _ : state) {
        benchmark::DoNotOptimize(fn(data.data(), n));
    }
    // Reported as items/s and bytes/s so the unroll/NEON wins are obvious.
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(n));
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(n * sizeof(int64_t)));
}

template <typename Fn>
void BM_Dot(benchmark::State &state, Fn fn) {
    const size_t n = static_cast<size_t>(state.range(0));
    const auto a = MakeF32(n, 1), b = MakeF32(n, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(fn(a.data(), b.data(), n));
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(n));
}

void BM_AddC(benchmark::State &state) {
    int64_t a = state.range(0), b = 7;
    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(add_i64_c(a, b));
    }
}

void BM_AddAsm(benchmark::State &state) {
    int64_t a = state.range(0), b = 7;
    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(add_i64_asm(a, b));
    }
}

}  // namespace

BENCHMARK(BM_AddC)->Arg(3);
BENCHMARK(BM_AddAsm)->Arg(3);

BENCHMARK_CAPTURE(BM_Sum, c, sum_i64_c)->RangeMultiplier(8)->Range(kMinN, kMaxN);
BENCHMARK_CAPTURE(BM_Sum, asm_scalar, sum_i64_asm)->RangeMultiplier(8)->Range(kMinN, kMaxN);
BENCHMARK_CAPTURE(BM_Sum, asm_unroll4, sum_i64_unroll4_asm)->RangeMultiplier(8)->Range(kMinN, kMaxN);
BENCHMARK_CAPTURE(BM_Sum, asm_neon, sum_i64_neon_asm)->RangeMultiplier(8)->Range(kMinN, kMaxN);

BENCHMARK_CAPTURE(BM_Dot, c, dot_f32_c)->RangeMultiplier(8)->Range(kMinN, kMaxN);
BENCHMARK_CAPTURE(BM_Dot, asm_neon, dot_f32_asm)->RangeMultiplier(8)->Range(kMinN, kMaxN);

BENCHMARK_MAIN();
