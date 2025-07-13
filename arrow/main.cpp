#include <arrow/compute/api_aggregate.h>
#include <arrow/compute/exec.h>
#include <iostream>
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <memory>
#include "perf-lib.hpp"



uint64_t sequential_sum(std::shared_ptr<arrow::Array> arr) {
    auto res = arrow::compute::CallFunction("sum", {arr});
    std::shared_ptr<arrow::Scalar> scalar = res->scalar();

    auto uint64_scalar = std::static_pointer_cast<arrow::UInt64Scalar>(scalar);
    return uint64_scalar->value;

}

std::shared_ptr<arrow::Array> build_arrow_uint64t(int sz) {
    arrow::UInt64Builder int64builder;
    for (int i = 0; i < sz; ++i) {
        int64builder.Append(i);
    }
    return int64builder.Finish().ValueOrDie();
}

int main() {

    for (int i = 0; i <= 23; ++i) {
        int sz = 1 << i;
        auto arr = build_arrow_uint64t(sz);

        PerfEventGroup events(PERF_COUNT_HW_CPU_CYCLES, PERF_TYPE_HARDWARE, "cycles");
        events.AddEvent(PERF_COUNT_HW_INSTRUCTIONS, PERF_TYPE_HARDWARE, "ins");

        asm volatile("mfence; lfence;" ::: "memory");
        events.Enable();
        std::cout << sequential_sum(arr) << std::endl;
        asm volatile("mfence; lfence;" ::: "memory");

        events.Disable();

        auto results = events.ReadEvents();
        std::cout << "vector size = " << sz << ", time = " << *results["wall_clock_ms"] << ", ipc = " << (double)*results["ins"]/(*results["cycles"]) << std::endl;
    }
    return 0;
}