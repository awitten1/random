// Reproduces Figure 2 of "MonetDB/X100: Hyper-Pipelining Query Execution"
// (Boncz, Zukowski, Nes; CIDR 2005): a selection primitive written with a
// branch vs. a predicated (branch-free) version, timed across selectivities.
//
// The input column holds uniform random ints in [0, 100), so `in[i] < V`
// selects V% of the rows. At 0% and 100% the branch is perfectly predictable;
// at 50% it is a coin flip and the branch version pays a mispredict on
// roughly every other row. The predicated version does the same work no
// matter what V is.
//
// Variant: clustered data. Match/no-match outcomes come from a two-state
// Markov chain whose switch probabilities are the iid ones divided by k:
//   P(match -> no match) = (1 - s) / k,   P(no match -> match) = s / k
// The stationary match rate is still s, but runs of each outcome are k times
// longer than in random data, so a "same as last time" predictor
// mispredicts at rate ~2s(1-s)/k. k = 1 is the paper's iid setup.
//
// Usage: ./sel_bench [n_rows] [reps] [k,k,...]   -> CSV on stdout
//   default k list: 1,4,16,64

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// noinline so each kernel is a standalone function we can disassemble
// (objdump -d) to confirm the branch version really has a conditional
// branch and the predicated version really doesn't.

__attribute__((noinline))
int sel_lt_int_col_int_val_branch(int n, int* __restrict res,
                                  const int* __restrict in, int V) {
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (in[i] < V)
            res[j++] = i;
    }
    return j;
}

__attribute__((noinline))
int sel_lt_int_col_int_val_pred(int n, int* __restrict res,
                                const int* __restrict in, int V) {
    int j = 0;
    for (int i = 0; i < n; i++) {
        bool b = (in[i] < V);
        res[j] = i;
        j += b;
    }
    return j;
}

// Fill `in` so that exactly the rows chosen by the Markov chain satisfy
// in[i] < V: matches are uniform in [0, V), non-matches uniform in [V, 100).
static void gen_clustered(vector<int>& in, int V, double k, mt19937& rng) {
    double s = V / 100.0;
    double p_leave_match = (1 - s) / k, p_leave_miss = s / k;
    uniform_real_distribution<double> u(0, 1);
    bool match = u(rng) < s;
    for (int& x : in) {
        x = match ? uniform_int_distribution<int>(0, V - 1)(rng)
                  : uniform_int_distribution<int>(V, 99)(rng);
        if (u(rng) < (match ? p_leave_match : p_leave_miss)) match = !match;
    }
}

using Kernel = int (*)(int, int*, const int*, int);

static double time_ms(Kernel k, int n, int* res, const int* in, int V,
                      int reps, int* out_count) {
    vector<double> ts;
    ts.reserve(reps);
    for (int r = 0; r < reps; r++) {
        auto t1 = chrono::steady_clock::now();
        // volatile store keeps the call from being treated as dead
        volatile int cnt = k(n, res, in, V);
        auto t2 = chrono::steady_clock::now();
        *out_count = cnt;
        ts.push_back(chrono::duration<double, milli>(t2 - t1).count());
    }
    // median is robust to the occasional context switch
    nth_element(ts.begin(), ts.begin() + reps / 2, ts.end());
    return ts[reps / 2];
}

int main(int argc, char** argv) {
    int n = argc > 1 ? atoi(argv[1]) : 1 << 24;
    int reps = argc > 2 ? atoi(argv[2]) : 11;
    vector<double> ks;
    {
        stringstream ss(argc > 3 ? argv[3] : "1,4,16,64");
        string tok;
        while (getline(ss, tok, ',')) ks.push_back(stod(tok));
    }

    vector<int> in(n), res(n);
    mt19937 rng(42);

    // Touch the output buffer once so page faults aren't billed to the
    // first measurement.
    sel_lt_int_col_int_val_pred(n, res.data(), in.data(), 100);

    printf("run_length,selectivity,branch_ms,predicated_ms,selected\n");
    for (double k : ks) {
        for (int V = 0; V <= 100; V += 10) {
            gen_clustered(in, V, k, rng);
            int cb = 0, cp = 0;
            double tb = time_ms(sel_lt_int_col_int_val_branch, n, res.data(),
                                in.data(), V, reps, &cb);
            double tp = time_ms(sel_lt_int_col_int_val_pred, n, res.data(),
                                in.data(), V, reps, &cp);
            if (cb != cp) {
                fprintf(stderr, "mismatch at k=%g V=%d: branch=%d pred=%d\n",
                        k, V, cb, cp);
                return EXIT_FAILURE;
            }
            printf("%g,%d,%.3f,%.3f,%d\n", k, V, tb, tp, cb);
            fflush(stdout);
        }
    }
    return 0;
}
