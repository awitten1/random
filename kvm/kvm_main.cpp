

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/kvm.h>
#include <unistd.h>

#include "kvm_vm.h"

int main() {
    Kvm::Initialize();
    Kvm::Vm vm;
    vm.SetMemory(0x1000);
    vm.CreateVCpu();
    
    return 0;
}


