
#include <algorithm>
#include <memory>
#include <string>
#include <err.h>
#include <iostream>
#include <stdlib.h>

struct Args {
    bool random; // sequential if not random.
    size_t bytes; // number of bytes to allocate.
};



Args parse_cli_args(int argc, char** argv) {
    Args ret{};
    int args = 0;
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--pattern") {
            std::string pattern(argv[i + 1]);
            if (i == argc - 1) {
                errx(EXIT_FAILURE, "must provide a pattern arg");
            }
            if (pattern == "random") {
                ret.random = true;
            } else {
                ret.random = false;
            }
            i++;
            args++;
        } else if (std::string(argv[i]) == "--bytes") {
            if (i == argc - 1) {
                errx(EXIT_FAILURE, "must provide a bytes arg");
            }
            ret.bytes = std::stoull(argv[i + 1]);
            args++;
            i++;
        } 
    }
    if (args != 2) {
        errx(EXIT_FAILURE, "must provide correct number of args");
    }
    return ret;
}

size_t sequential_pattern(long* buf, size_t sz) {
    size_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += buf[i];
    }
    return ret;
}

size_t random_pattern(long* buf, size_t sz) {
    size_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += buf[rand() % sz];
    }
    return ret;
}

int main(int argc, char** argv) {
    Args args = parse_cli_args(argc, argv);
    std::cout << args.random << ", " << args.bytes << std::endl;
    size_t num_ints = args.bytes / sizeof(long);
    std::unique_ptr<long[]> ptr(new long[num_ints]);   
    std::generate(ptr.get(), ptr.get() + num_ints, []() { return rand(); });
    size_t sum;
    if (args.random) {
        sum = random_pattern(ptr.get(), num_ints);
    } else {
        sum = sequential_pattern(ptr.get(), num_ints);
    }
    std::cout << sum << std::endl;
    return 0;
}
