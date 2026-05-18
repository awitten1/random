
#include <algorithm>
#include <cstdio>
#include <vector>
#include <thrust/execution_policy.h>
#include <thrust/universal_vector.h>
#include <thrust/transform.h>

int main() {
    float k = 0.5;
    float ambient_temp = 20;
    // std::vector<float> temp{42,24,50};
    thrust::universal_vector<float> temp{ 42, 24, 50 };
    auto transformation = [=] __host__ __device__ (float temp) { return temp + k * (ambient_temp - temp); };

    std::printf("step\ttemp[0]\ttemp[1]\ttemp[2]\n");

    for (int step = 0; step < 3; ++step) {
        // std::transform(temp.begin(), temp.end(), temp.begin(), [=](float temp) {
        //     return temp + k*(ambient_temp - temp);
        // });
        thrust::transform(temp.begin(), temp.end(), temp.begin(), transformation);
        std::printf("%d\t%.2f\t%.2f\t%.2f\n",
            step, temp[0], temp[1], temp[2]);
    }

    return 0;
}
