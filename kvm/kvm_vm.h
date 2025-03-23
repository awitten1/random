



#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/kvm.h>
#include <vector>


namespace Kvm {

class Fd {

    int fd_ = -1;

public:

    int fd() { return fd_; }

    operator int() {
        return fd_;
    }

    static Fd Open(const char* c_str, int flags) {
        int fd = open(c_str, flags);
        return Fd(fd);
    }

    Fd() = default;

    Fd(int fd) : fd_(fd) {
    }

    Fd(const Fd&) = delete;

    Fd(Fd&& other) {
        fd_ = other.fd_;
        other.fd_ = -1;
    }

    void set_fd(int fd) {
        if (fd_ > 0) {
            close(fd_);
        }
        fd_ = fd;
    }

    ~Fd() {
        if (fd_ <= 0) return;
        close(fd_);
    }
};

void Initialize();


class Vm {

    Fd vm_fd_;
    int sys_fd_;
    std::vector<Fd> cpu_fd_;

public:

    explicit Vm();

    void SetMemory(size_t bytes);
    void SetMemory(void* buf);

    void CreateVCpu();

};


}

