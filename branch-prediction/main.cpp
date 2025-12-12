// try with fno-if-conversion and -fno-tree-vectorize
#include <csignal>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <random>
#include <unistd.h>

using namespace std;

vector<unsigned char> gen_vec(bool sorted) {
    auto r = []() {
        static std::random_device r;
        static std::default_random_engine e1(r());
        static std::uniform_int_distribution<int> uniform_dist(1, 256);
        return uniform_dist(e1);
    };
    vector<unsigned char> ret;
    for (int i = 0; i < 1e8; ++i) {
        ret.push_back(r()%256);
    }
    if (sorted) {
        sort(ret.begin(), ret.end());
    }
    return ret;
}


pair<uint32_t,uint32_t> sums(const vector<unsigned char>& vec) {
    uint32_t sum1 = 0, sum2 = 0;
    for (unsigned char c : vec) {
        if (c < 128) {
            sum1 += c;
        }
        sum2+=c;
    }
    return {sum1, sum2};
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "./main <sorted|unsorted>" << endl;
        exit(EXIT_FAILURE);
    }
    auto vec = gen_vec(string(argv[1]) == "sorted");

    int pid = getpid();
    int cpid = fork();
    if (cpid == 0) {
        char buf[50];
        sprintf(buf, "perf stat -p %d   > stat.log 2>&1",pid);
        execl("/bin/sh", "sh", "-c", buf, NULL);
        return 0;
    }

    setpgid(cpid, 0);


    auto t1 =  chrono::steady_clock::now();
    volatile auto ret = sums(vec);
    auto t2 = chrono::steady_clock::now();
    chrono::duration<double,milli> diff = t2 - t1;
    cout << diff.count() << endl;

    ::kill(-cpid, SIGINT);
    return 0;
}
