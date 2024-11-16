
#include <cstring>
#include <elf.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

// ELF Reader based on https://man7.org/linux/man-pages/man5/elf.5.html.

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

// Only meaningful for executable and shared object files.
//  readelf --program-headers <object file>
//  or readelf --segments <object file>
//
// "An executable or shared object file's program header table is an
// array of structures, each describing a segment or other
// information the system needs to prepare the program for
// execution.  An object file segment contains one or more sections.
// Program headers are meaningful only for executable and shared
// object files.  A file specifies its own program header size with
// the ELF header's e_phentsize and e_phnum members.  The ELF
// program header is described by the type Elf32_Phdr or Elf64_Phdr
// depending on the architecture:"
void ReadProgramHeaderTable(Elf64_Ehdr x, int fd) {

  std::cout << "--------------------" << std::endl;
  std::cout << "Program Header Table" << std::endl;
  std::cout << "--------------------" << std::endl;

  for (uint64_t i = 0; i < x.e_phnum; ++i) {
    uint64_t offset = i * x.e_phentsize + x.e_phoff;

    Elf64_Phdr phdr;
    if (sizeof(phdr) != x.e_phentsize) {
      throw std::runtime_error{"Elf64_Phdr size does not match x.e_phentsize"};
    }

    int ret = pread(fd, &phdr, x.e_phentsize, offset);
    if (ret != x.e_phentsize) {
      throw std::runtime_error{"failed to read program header"};
    }

    switch(phdr.p_type) {
      case PT_NULL:
        break;
      case PT_LOAD:
        std::cout << "PT_LOAD: loadable segment" << std::endl;
        break;
      case PT_DYNAMIC:
        std::cout << "PT_DYNAMIC: dynamic linking info" << std::endl;
        break;
      case PT_INTERP:
        std::cout << "PT_INTERP: interpreter info!" << std::endl;
        break;
      case PT_NOTE:
        std::cout << "PT_NOTE: notes" << std::endl;
        break;
      case PT_SHLIB:
        std::cout << "PT_SHLIB" << std::endl;
        break;
      case PT_GNU_EH_FRAME:
        std::cout << "PT_GNU_EH_FRAME" << std::endl;
        break;
      case PT_PHDR:
        std::cout << "PT_PHDR: program header table location entry" << std::endl;
        break;
      case PT_LOPROC:
        std::cout << "PT_LOPROC: processor specific low" << std::endl;
        break;
      case PT_HIPROC:
        std::cout << "PT_HIPROC: processor specific high" << std::endl;
        break;
      case PT_GNU_STACK:
        std::cout << "PT_GNU_STACK" << std::endl;
        break;
    }

  }
}

void ReadSectionHeaderTable(Elf64_Ehdr x, int fd) {

  std::cout << "--------------------" << std::endl;
  std::cout << "Section Header Table" << std::endl;
  std::cout << "--------------------" << std::endl;

  for (uint64_t i = 0; i < x.e_shnum; ++i) {
    Elf64_Shdr shdr;
    uint64_t offset = i * x.e_shentsize + x.e_shoff;
    if (sizeof(shdr) != x.e_shentsize) {
      throw std::runtime_error{"Elf64_Shdr size does not match x.e_shentsize"};
    }

    int ret = pread(fd, &shdr, x.e_shentsize, offset);
    if (ret != x.e_shentsize) {
      throw std::runtime_error{"failed to read section header"};
    }

    switch (shdr.sh_type) {
    case SHT_NULL:
      std::cout << "SHT_NULL: It does not have an associated section." << std::endl;
      break;
    case SHT_PROGBITS:
      std::cout << "SHT_PROGBITS: This section holds information defined by the "
                   "program, whose format and meaning are determined "
                   "solely by the program. " << std::endl;
      break;

    case SHT_SYMTAB:
      std::cout << "SHT_SYMTAB: "
        "This section holds a symbol table.  Typically, \n"
        "SHT_SYMTAB provides symbols for link editing, \n"
        "though it may also be used for dynamic linking.  As \n"
        "a complete symbol table, it may contain many \n"
        "symbols unnecessary for dynamic linking.  An object \n"
        "file can also contain a SHT_DYNSYM section. \n" << std::endl;
      break;

    case SHT_STRTAB:
      std::cout << "SHT_STRTAB: This section holds a string table.  An object file "
                   "may have multiple string table sections." << std::endl;
      break;

    case SHT_RELA:
      std::cout << "SHT_RELA: "
                   "This section holds relocation entries with explicit "
                   "addends, such as type Elf32_Rela for the 32-bit "
                   "class of object files.  An object may have multiple "
                   "relocation sections. " << std::endl;
      break;

    case SHT_HASH:
      std::cout << "SHT_HASH: This section holds a symbol hash table.  An object "
                   "participating in dynamic linking must contain a "
                   "symbol hash table.  An object file may have only "
                   "one hash table. " << std::endl;
      break;

    case SHT_DYNAMIC:
      std::cout << "SHT_DYNAMIC: This section holds information for dynamic linking. "
                   "An object file may have only one dynamic section. " << std::endl;
      break;

    case SHT_NOTE:
      std::cout << "SHT_NOTE: This section holds notes (ElfN_Nhdr)." << std::endl;
      break;

    case SHT_NOBITS:
      std::cout << "SHT_NOBITS: A section of this type occupies no space in the "
                   "file but otherwise resembles SHT_PROGBITS. "
                   "Although this section contains no bytes, the "
                   "sh_offset member contains the conceptual file "
                   "offset. " << std::endl;
      break;

    case SHT_REL:
      std::cout << "SHT_REL: This section holds relocation offsets without "
                   "explicit addends, such as type Elf32_Rel for the "
                   "32-bit class of object files.  An object file may "
                   "have multiple relocation sections. " << std::endl;
      break;
    case SHT_SHLIB:
      std::cout << "SHT_SHLIB: This section is reserved but has unspecified "
                   "semantics." << std::endl;
      break;

    case SHT_DYNSYM:
      std::cout << "SHT_DYNSYM: This section holds a minimal set of dynamic linking "
                   "symbols.  An object file can also contain a "
                  "SHT_SYMTAB section. " << std::endl;
      break;

    case SHT_LOPROC:
      std::cout << "SHT_LOPROC" << std::endl;
      break;
    case SHT_HIPROC:
      std::cout << "SHT_HIPROC" << std::endl;
      break;

    case SHT_LOUSER:
      std::cout << "SHT_LOUSER: This value specifies the lower bound of the range "
                   "of indices reserved for application programs." << std::endl;
      break;
    case SHT_HIUSER:
      std::cout << "SHT_HIUSER: This value specifies the upper bound of the range "
                   "of indices reserved for application programs. "
                   "Section types between SHT_LOUSER and SHT_HIUSER may "
                   "be used by the application, without conflicting "
                   "with current or future system-defined section "
                   "types." << std::endl;
      break;
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

  // Reading headers.
  // readelf -h <object file>
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
  ReadSectionHeaderTable(x, fd);

  return 0;
}