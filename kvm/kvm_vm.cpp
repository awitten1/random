

#include "kvm_vm.h"
#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <sys/mman.h>

namespace Kvm {

static Fd kvm_fd = -1;

Vm::Vm() : sys_fd_(kvm_fd) {
    sys_fd_ = kvm_fd;
    int fd = ioctl(sys_fd_, KVM_CREATE_VM, KVM_X86_DEFAULT_VM);
    if (fd < 0) {
        std::ostringstream oss;
        oss << "failed to create vm.  reason = " << strerror(errno) << std::endl;
        throw std::runtime_error(oss.str());
    }
    vm_fd_.set_fd(fd);
}

void Initialize() {
    int fd = open("/dev/kvm", O_RDWR);
    if (fd < 0) {
        std::ostringstream oss;
        oss << "failed to open /dev/kvm.  reason = " << strerror(errno) << std::endl;
        throw std::runtime_error(oss.str());
    }

    kvm_fd.set_fd(fd);

    int api_version = ioctl(kvm_fd, KVM_GET_API_VERSION, 0);
    if (api_version != 12) {
        std::ostringstream oss;
        oss << "api version should be 12 but was " << api_version << std::endl;
        throw std::runtime_error(oss.str());
    }

    int ret = ioctl(kvm_fd,  KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY);
    if (ret <= 0) {
        std::ostringstream oss;
        oss << "KVM_CAP_USER_MEMORY extension not supported" << std::endl;
        throw std::runtime_error(oss.str());
    }
}

void Vm::CreateVCpu() {
    int vcpufd = ioctl(vm_fd_, KVM_CREATE_VCPU, (unsigned long)0);
    if (vcpufd == -1) {
        std::ostringstream oss;
        oss << "failed to create cpu. reason = " << strerror(errno) << std::endl;
        throw std::runtime_error(oss.str());
    }
    cpu_fd_.push_back(vcpufd);
}

void Vm::SetMemory(size_t bytes) {
    void* mem = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .guest_phys_addr = 0x1000,
        .memory_size = 0x1000,
        .userspace_addr = (uint64_t)mem,
    };
    int ret = ioctl(vm_fd_, KVM_SET_USER_MEMORY_REGION, &region);
    if (ret == -1) {
        std::ostringstream oss;
        oss << "failed to add memory.  reason = " << strerror(errno) << std::endl;
        throw std::runtime_error(oss.str());
    }
}

}