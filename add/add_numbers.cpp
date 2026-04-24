
#include <iostream>
#include <numeric>
#include <vector>

__attribute__((noinline))
long add(long* nums, long sz) {
    long ret = 0;
    for (long i = 0; i < sz; ++i) {
        ret += nums[i];
    }
    return ret;
}


__attribute__((noinline))
long add_unrolled(long* nums, long sz) {
    long ret[4] = {0,0,0,0};
    long i = 0;
    for (; i < sz-4; i+=4) {
        ret[0] += nums[i];
        ret[1] += nums[i+1];
        ret[2] += nums[i+2];
        ret[3] += nums[i+3];
    }
    long retn = ret[0] + ret[1] + ret[2] + ret[3];
    for (; i < sz; ++i) {
        retn += nums[i];
    }
    return retn;
}


int main(int argc, char** argv) {
    long sz = std::stol(argv[1]);
    std::vector<long> nums(sz);
    std::iota(nums.begin(), nums.end(), 0);
    int reps = std::stoi(argv[2]);

    volatile long ret = 0;
    for (int i = 0; i < reps; ++i) {
        #ifndef UNROLLED
        ret = add(nums.data(), nums.size());
        #else
        ret = add_unrolled(nums.data(), nums.size());
        #endif
    }
    std::cout << ret << std::endl;
    return 0;

}