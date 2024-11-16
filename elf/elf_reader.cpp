
#include <cstring>
#include <elf.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>


void validateElfN_Ehdr(Elf64_Ehdr x) {
  if (x.e_ident[EI_MAG0] != ELFMAG0) {
    throw std::runtime_error{"e_ident 0 was invalid"};
  }
  if (x.e_ident[EI_MAG1] != ELFMAG1) {
    throw std::runtime_error{"e_ident 1 was invalid"};
  }
  if (x.e_ident[EI_MAG2] != ELFMAG2) {
    throw std::runtime_error{"e_ident 2 was invalid"};
  }
  if (x.e_ident[EI_MAG3] != ELFMAG3) {
    throw std::runtime_error{"e_ident 3 was invalid"};
  }
}

void checkAndLogArchitecture(Elf64_Ehdr x) {
  switch (x.e_ident[EI_CLASS]) {
  case ELFCLASSNONE:
    std::cerr << "invalid arch for binary" << std::endl;
    throw std::runtime_error{"invalid arch for binary"};
  case ELFCLASS32:
    std::cout << "32-bit architecture." << std::endl;
    break;
  case ELFCLASS64:
    std::cout << "64-bit architecture." << std::endl;
    break;
  }
}

void logEncoding(Elf64_Ehdr x) {
  switch (x.e_ident[EI_DATA]) {
  case ELFDATANONE:
    std::cerr << "unknown data format" << std::endl;
    break;
  case ELFDATA2LSB:
    std::cout << "Two's complement, little-endian." << std::endl;
    break;
  case ELFDATA2MSB:
    std::cout << "Two's complement, big-endian." << std::endl;
    break;
  }
}

void log(std::string_view sv) {
  std::cout << sv << std::endl;
}


void logOsAbi(Elf64_Ehdr x) {
  switch (x.e_ident[EI_OSABI]) {
  case ELFOSABI_SYSV:
    log("UNIX System V ABI");
    break;
  case ELFOSABI_HPUX:
    log("HP-UX ABI");
    break;
  case ELFOSABI_NETBSD:
    log("NetBSD ABI");
    break;
  case ELFOSABI_LINUX:
    log("Linux ABI");
    break;
  case ELFOSABI_SOLARIS:
    log("Solaris ABI");
    break;
  case ELFOSABI_IRIX:
    log("IRIX ABI");
    break;
  case ELFOSABI_FREEBSD:
    log("FreeBSD ABI");
    break;
  case ELFOSABI_TRU64:
    log("TRU64 UNIX ABI");
    break;
  case ELFOSABI_ARM:
    log("ARM architecture ABI");
    break;
  case ELFOSABI_STANDALONE:
    log("Stand-alone (embedded) ABI");
    break;
  }
}

void logObjectFileType(Elf64_Ehdr x) {
  switch (x.e_type) {
  case ET_NONE:
    log("An unknown type.");
    break;
  case ET_REL:
    log("A relocatable file.");
    break;
  case ET_EXEC:
    log("An executable file.");
    break;
  case ET_DYN:
    log("A shared object.");
    break;
  case ET_CORE:
    log("A core file.");
    break;
  }
}


void logMachine(Elf64_Ehdr x) {
  std::cout << x.e_machine << std::endl;
  switch(x.e_machine) {
  case EM_NONE:
    log("An unknown machine");
    break;
  case EM_M32:
    log("AT&T WE 32100");
    break;
  case EM_SPARC:
    log("Sun Microsystems SPARC");
    break;
  case EM_386:
    log("Intel 80386");
    break;
  case EM_68K:
    log("Motorola 68000");
    break;
  case EM_88K:
    log("Motorola 88000");
    break;
  case EM_860:
    log("Intel 80860");
    break;
  case EM_MIPS:
    log("MIPS RS3000 (big-endian only)");
    break;
  case EM_PARISC:
    log("HP/PA");
    break;
  case EM_SPARC32PLUS:
    log("SPARC with enhanced instruction set");
    break;
  case EM_PPC:
    log("PowerPC");
    break;
  case EM_PPC64:
    log("PowerPC 64-bit");
    break;
  case EM_S390:
    log("IBM S/390");
    break;
  case EM_ARM:
    log("Advanced RISC Machines");
    break;
  case EM_SH:
    log("Renesas SuperH");
    break;
  case EM_SPARCV9:
    log("SPARC v9 64-bit");
    break;
  case EM_IA_64:
    log("Intel Itanium");
    break;
  case EM_X86_64:
    log("AMD x86-64");
    break;
  case EM_VAX:
    log("DEC Vax");
    break;
  case EM_AARCH64:
    log("AARCH64");
    break;
  }
}

void logEntryPointAddress(Elf64_Ehdr x) {
  printf("Virtual Memory Address Entrypoint: 0x%lx\n", x.e_entry);
}

void logProgramHeaderFileOffset(Elf64_Ehdr x) {
  printf("program header file offset: %lu (bytes)\n", x.e_phoff);
}


void logSectionHeaderFileOffset(Elf64_Ehdr x) {
  printf("section header file offset: %lu (bytes)\n", x.e_shoff);
}

void logSizeOfHeader(Elf64_Ehdr x) {
  printf("Size of this header: %d.  Sizeof structure: %lu\n", x.e_ehsize, sizeof(x));
}

uint64_t GetProgramHeaderOffset(Elf64_Ehdr x) {
  return x.e_phoff;
}

uint64_t EntriesInProgramHeader(Elf64_Ehdr x) {
  return x.e_phnum;
}

uint64_t ProgramHeaderEntrySize(Elf64_Ehdr x) {
  return x.e_phentsize;
}

void ReadProgramHeaderTable(Elf64_Ehdr x, int fd) {
  for (uint64_t i = 0; i < x.e_phnum; ++i) {
    uint64_t offset = i * x.e_phentsize + x.e_phoff;

    char* buf = new char[x.e_phentsize];
    int ret = pread(fd, buf, x.e_phentsize, offset);
    std::cout << x.e_phentsize << ", " << sizeof(Elf64_Phdr) << std::endl;
    if (ret != x.e_phentsize) {
      throw std::runtime_error{"failed to read program header"};
    }
  }
}

int main(int argc, char** argv) {
  Elf64_Ehdr x;
  memset(&x, 0, sizeof(x));

  std::string filename = argv[1];

  int ret;
  int fd = open(filename.data(), O_RDONLY);

  if (fd == -1) {
    throw std::runtime_error{strerror(errno)};
  }

  ret = read(fd, &x, sizeof(x));
  if (ret != sizeof(x)) {
    throw std::runtime_error{strerror(errno)};
  }

  validateElfN_Ehdr(x);
  checkAndLogArchitecture(x);
  logEncoding(x);
  logOsAbi(x);
  logObjectFileType(x);
  logMachine(x);
  logEntryPointAddress(x);
  logProgramHeaderFileOffset(x);
  logSectionHeaderFileOffset(x);
  logSizeOfHeader(x);

  ReadProgramHeaderTable(x, fd);

  return 0;
}