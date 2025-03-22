

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/kvm.h>
#include <unistd.h>

class Fd {

    int fd_;
    bool moved_ = false;

public:

    int fd() { return fd_; }

    operator int() {
        return moved_ ? -1 : fd_;
    }

    static Fd Open(const char* c_str, int flags) {
        int fd = open(c_str, flags);
        return Fd(fd);
    }

    Fd(int fd) : fd_(fd) {
    }

    Fd(Fd&& other) {
        fd_ = other.fd_;
        moved_ = false;
        other.moved_ = true;
    }

    Fd(const Fd&) = delete;

    ~Fd() {
        if (moved_) return;
        close(fd_);
    }
};

int main() {
    Fd fd = Fd::Open("/dev/kvm", O_RDWR);
    if (fd < 0) {
        std::cerr << "failed to open /dev/kvm. reason = " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    int api_version = ioctl(fd, KVM_GET_API_VERSION, 0);
    if (api_version != 12) {
        // https://github.com/torvalds/linux/blob/88d324e69ea9f3ae1c1905ea75d717c08bdb8e15/Documentation/virt/kvm/api.rst#4-api-description
        std::cerr << "KVM_GET_API_VERSION ioctl must return 12. reason = " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    int supported_vm_types_bitmap = ioctl(fd, KVM_CHECK_EXTENSION, KVM_CAP_VM_TYPES);
    if (supported_vm_types_bitmap == -1) {
        std::cerr << "kvm get vm types not supported" << std::endl;
        return EXIT_FAILURE;
    }

    if (!(supported_vm_types_bitmap & (1 << KVM_X86_DEFAULT_VM))) {
        std::cerr << "KVM_X86_DEFAULT_VM not supported" << std::endl;
    }

    if (!(supported_vm_types_bitmap & (1 << KVM_X86_SW_PROTECTED_VM))) {
        std::cerr << "KVM_X86_SW_PROTECTED_VM not supported" << std::endl;
    }

    int ret = ioctl(fd, KVM_CREATE_VM, KVM_X86_DEFAULT_VM);
    if (ret == -1) {
        std::cerr << "failed to create VM. reason = " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}


