
#include <chrono>
#include <cstdint>
#include <iostream>
#include <ratio>
#include <sstream>
#include <thread>
#include <atomic>
#include <unistd.h>

struct X {
#ifdef PADDED
    alignas(64) std::atomic<int64_t> x = 0;
    alignas(64) std::atomic<int64_t> y = 0;
#else
    std::atomic<int64_t> x = 0;
    std::atomic<int64_t> y = 0;
#endif
};

X g;

const auto& now = std::chrono::steady_clock::now;

int main() {
    auto l = [](std::atomic<int64_t>& a) {
        auto start = now();
        for (;;) {
            int64_t num = ++a;
            if ((num & (num - 1)) == 0) {
                std::ostringstream os;
                os << "reached " << num << " in time " <<
                    std::chrono::duration<float, std::milli>(now() - start).count() << " ms" << std::endl;
                std::cout << os.str();
            }
        }
    };

    std::thread t([l]() { l(g.x); });
    std::thread s([l]() { l(g.y); });

    t.detach();
    s.detach();

    pause();

    return 0;
}