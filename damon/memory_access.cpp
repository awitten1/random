
#include <string>
#include <err.h>
#include <iostream>


struct Args {
    bool random; // sequential if not random.
    size_t bytes; // number of bytes to allocate.
};


Args parse_cli_args(int argc, char** argv) {
    Args ret{};
    int args = 0;
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--pattern") {
            std::string pattern(argv[i+1]);
            if (i == argc - 1) {
                errx(EXIT_FAILURE, "must provide a pattern arg");
            }
            if (pattern == "random") {
                ret.random = true;
            } else {
                ret.random = false;
            }
            args++;
        } else if (std::string(argv[i]) == "--bytes") {
            if (i == argc - 1) {
                errx(EXIT_FAILURE, "must provide a bytes arg");
            }
            ret.bytes = std::stoull(argv[i + 1]);
            args++;
        }
    }
    if (args != 2) {
        errx(EXIT_FAILURE, "must provide correct number of args");
    }
    return ret;
}


int main(int argc, char** argv) {
    Args args = parse_cli_args(argc, argv);
    std::cout << args.random << ", " << args.bytes << std::endl;
    return 0;
}
