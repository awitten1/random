
#include <chrono>
#include <cstdint>
#include <iostream>
#include <ratio>
#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <unistd.h>


struct X {
    alignas(64) int64_t x = 0;
    alignas(64) int64_t y = 0;
};

struct Y {
    int64_t x = 0;
    int64_t y = 0;
};



X g1{};
Y g2{};

const auto& now = std::chrono::steady_clock::now;

int main(int argc, char** argv) {
    long x = std::stol(argv[1]);
    auto l = [x](volatile int64_t& a) {
        for (int i = 0; i < x; ++i) {
            int64_t num = ++a;
        }
    };

    auto start = now();

    std::thread t([l]() { l(g1.x); });
    std::thread s([l]() { l(g1.y); });

    t.join();
    s.join();

    auto end = now();
    std::cout << std::chrono::duration<double>(end - start).count() << " seconds\n";

    start = now();

    std::thread t1([l]() { l(g2.x); });
    std::thread s1([l]() { l(g2.y); });

    t1.join();
    s1.join();

    end = now();
    std::cout << std::chrono::duration<double>(end - start).count() << " seconds\n";

    return 0;
}