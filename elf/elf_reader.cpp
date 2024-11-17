
#include <cassert>
#include <cstdint>
#include <cstring>
#include <elf.h>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <unordered_map>
#include <vector>
#include <cxxabi.h>
#include <sys/stat.h>
#include <span>

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

void logIndexOfStringTableInSectionHeaderTable(Elf64_Ehdr x) {
  printf("Index of string table. e_shstrndx = %d\n", x.e_shstrndx);
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

std::string demangle(const std::string& mangled_name) {
  int status;
  char* realname = abi::__cxa_demangle(mangled_name.c_str(), NULL, NULL, &status);
  if (realname == nullptr) {
    return "";
  }
  std::string ret = realname;
  free(realname);
  return ret;
}

class ElfStrings {
public:
  ElfStrings(Elf64_Ehdr hdr, int fd) : fd_(fd), hdr_(hdr) {}

  void ReadStringTable(Elf64_Shdr shdr, uint32_t idx) {
    if (shdr.sh_type != SHT_STRTAB) {
      throw std::logic_error{"must provide SHT_STRTAB section header entry."};
    }

    std::unique_ptr<char[]> buf(new char[shdr.sh_size]);
    int ret = pread(fd_, buf.get(), shdr.sh_size, shdr.sh_offset);
    if (ret != shdr.sh_size) {
      throw std::runtime_error{"failed to read entire string table"};
    }

    uint32_t off = 0;
    char* view = buf.get();

    m_[idx] = std::move(buf);

  }

  std::string GetStringForSymbol(Elf64_Sym sym, uint32_t sh_link_for_symtab) const {
    auto it = m_.find(sh_link_for_symtab);
    if (it == m_.end()) {
      throw std::runtime_error{"could not find strtab for symtab."};
    }
    char* buf = it->second.get();
    return std::string(&buf[sym.st_name]);
  }

  std::string LookupHeaderName(uint32_t name) const {
    auto it = m_.find(GetIdxOfSectionHeaderStringTable());
    assert(it != m_.end());
    return std::string(&it->second[name]);
  }

private:
  int fd_;
  Elf64_Ehdr hdr_;

  uint32_t GetIdxOfSectionHeaderStringTable() const {
    return hdr_.e_shstrndx;
  }

  // maps section header index -> in memory string table.
  // Only defined for .strtab sections.
  std::unordered_map<uint32_t, std::unique_ptr<char[]>> m_;
};

std::string GetVisibility(Elf64_Sym sym) {
  auto vis = ELF64_ST_VISIBILITY(sym.st_other);
  switch(vis) {
  case STV_DEFAULT:
    return "default";
  case STV_INTERNAL:
    return "internal";
  case STV_HIDDEN:
    return "hidden";
  case STV_PROTECTED:
    return "protected";
  }
  throw std::runtime_error{"unreachable"};
}

std::string GetBinding(Elf64_Sym sym) {
  auto bind = ELF32_ST_BIND(sym.st_info);
  switch(bind) {
  case STB_LOCAL:
    return "local";
  case STB_GLOBAL:
    return "global";
  case STB_WEAK:
    return "weak";
  case STB_GNU_UNIQUE:
    return "unique";
  }
  throw std::runtime_error{"unhandled binding for symbol"};
}

std::string GetType(Elf64_Sym sym) {
  auto type = ELF32_ST_TYPE(sym.st_info);
  switch(type) {
  case STT_NOTYPE:
    return "NOTYPE";
  case STT_OBJECT:
    return "DATA_OBJECT";
  case STT_FUNC:
    return "FUNC";
  case STT_SECTION:
    return "SECTION";
  case STT_FILE:
    return "FILE";
  case STT_COMMON:
    return "COMMON";
  case STT_TLS:
    return "TLS";
  case STT_GNU_IFUNC:
    return "INDIRECT";
  }
  throw std::runtime_error{"unhandled type for symbol"};
}

void ReadSymbolTable(Elf64_Shdr shdr, int fd, const ElfStrings& efs) {
  if (shdr.sh_type != SHT_SYMTAB) {
    throw std::logic_error{"Must provide a section header for a symbol table."};
  }


  std::unique_ptr<char[]> buf(new char[shdr.sh_size]);
  int ret = pread(fd, buf.get(), shdr.sh_size, shdr.sh_offset);
  if (ret != shdr.sh_size) {
    throw std::runtime_error{"failed to read symbol table"};
  }

  Elf64_Sym* sym_array = (Elf64_Sym*)buf.get();

  assert(shdr.sh_size % shdr.sh_entsize == 0);
  uint32_t num_entries = shdr.sh_size / shdr.sh_entsize;

  for (uint32_t i = 0; i < num_entries; ++i) {
    if (sym_array[i].st_name == 0) {
      continue;
    }
    // https://docs.oracle.com/cd/E19683-01/816-1386/6m7qcoblj/index.html#chapter6-47976
    // sh_link for symtab sections references the needed strtab section.
    std::string str = efs.GetStringForSymbol(sym_array[i], shdr.sh_link);
    std::string vis = GetVisibility(sym_array[i]);
    std::string binding = GetBinding(sym_array[i]);
    std::string type = GetType(sym_array[i]);
    printf("%016lx %s %s %s\n", sym_array[i].st_value, type.c_str(), vis.c_str(), str.c_str());
  }

}


class ElfFile {
public:
  ElfFile(const std::string& file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    if (fd == -1) {
      throw std::runtime_error{"failed to open file"};
    }

    struct stat st;
    int ret = fstat(fd, &st);
    if (ret == -1) {
      throw std::runtime_error{"failed to stat file"};
    }

    sz_ = st.st_size;

    ptr_ = mmap(NULL, sz_, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr_ == MAP_FAILED) {
      throw std::runtime_error{"failed to map file into memory"};
    }

    ValidateHeader();

  }

  void ValidateHeader() {
    Elf64_Ehdr* hdr = (Elf64_Ehdr*)ptr_;
    validateElfN_Ehdr(*hdr);
  }

  ~ElfFile() {
    munmap(ptr_, sz_);
  }



private:

  Elf64_Ehdr* getElfHeader() {
    return (Elf64_Ehdr*)ptr_;
  }

  std::span<Elf64_Shdr> getSectionHeaders() {
    auto elf_header = getElfHeader();
    Elf64_Shdr* shdrs = (Elf64_Shdr*)((char*)ptr_ + elf_header->e_shoff);
    return std::span{shdrs, elf_header->e_shnum};
  }

  void* ptr_;
  size_t sz_;
};

ElfStrings BuildStringTable(Elf64_Ehdr hdr, int fd) {
  ElfStrings es(hdr, fd);
  int seen = 0;
  for (uint64_t i = 0; i < hdr.e_shnum; ++i) {
    Elf64_Shdr shdr;
    uint64_t offset = i * hdr.e_shentsize + hdr.e_shoff;
    if (sizeof(shdr) != hdr.e_shentsize) {
      throw std::runtime_error{"Elf64_Shdr size does not match x.e_shentsize"};
    }

    int ret = pread(fd, &shdr, hdr.e_shentsize, offset);
    if (ret != hdr.e_shentsize) {
      throw std::runtime_error{"failed to read section header"};
    }

    if (shdr.sh_type == SHT_STRTAB) {
      es.ReadStringTable(shdr, i);
    }

  }

  return es;
}

void ReadSectionHeaderTable(Elf64_Ehdr x, int fd) {

  // std::cout << "--------------------" << std::endl;
  // std::cout << "Section Header Table" << std::endl;
  // std::cout << "--------------------" << std::endl;

  ElfStrings strtab = BuildStringTable(x, fd);

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
      // std::cout << "SHT_NULL" << std::endl;
      break;
    case SHT_PROGBITS:
      // std::cout << "SHT_PROGBITS" << std::endl;
      break;

    case SHT_STRTAB:
      // std::cout << "SHT_STRTAB" << std::endl;
      break;

    case SHT_SYMTAB:
      // std::cout << "SHT_SYMTAB" << std::endl;
      ReadSymbolTable(shdr, fd, strtab);
      break;

    case SHT_RELA:
      // std::cout << "SHT_RELA" << std::endl;
      break;

    case SHT_HASH:
      // std::cout << "SHT_HASH" << std::endl;
      break;

    case SHT_DYNAMIC:
      // std::cout << "SHT_DYNAMIC" << std::endl;
      break;

    case SHT_NOTE:
      // std::cout << "SHT_NOTE" << std::endl;
      break;

    case SHT_NOBITS:
      // std::cout << "SHT_NOBITS" << std::endl;
      break;

    case SHT_REL:
      // std::cout << "SHT_REL" << std::endl;
      break;
    case SHT_SHLIB:
      // std::cout << "SHT_SHLIB" << std::endl;
      break;

    case SHT_DYNSYM:
      // std::cout << "SHT_DYNSYM" << std::endl;
      break;

    case SHT_LOPROC:
      // std::cout << "SHT_LOPROC" << std::endl;
      break;
    case SHT_HIPROC:
      // std::cout << "SHT_HIPROC" << std::endl;
      break;

    case SHT_LOUSER:
      // std::cout << "SHT_LOUSER" << std::endl;
      break;
    case SHT_HIUSER:
      // std::cout << "SHT_HIUSER" << std::endl;
      break;
    case SHT_INIT_ARRAY:
      // std::cout << "SHT_INIT_ARRAY" << std::endl;
      break;
    case SHT_FINI_ARRAY:
      // std::cout << "SHT_FINI_ARRAY" << std::endl;
      break;
    }

    // std::cout << "name = " << strtab.LookupHeaderName(shdr.sh_name) << std::endl;

    // bool writable_during_execution = shdr.sh_flags & SHF_WRITE;
    // bool contains_executable_ins = shdr.sh_flags & SHF_EXECINSTR;
    // bool occupies_memory_during_exec = shdr.sh_flags & SHF_ALLOC;

  }
}

int main(int argc, char** argv) {
  // Elf64_Ehdr x;
  // memset(&x, 0, sizeof(x));

  std::string filename = argv[1];

  ElfFile elf_file(filename);
  elf_file.ValidateHeader();

  // Reading headers.
  // readelf -h <object file>
  // validateElfN_Ehdr(x);
  // checkAndLogArchitecture(x);
  // logEncoding(x);
  // logOsAbi(x);
  // logObjectFileType(x);
  // logMachine(x);
  // logEntryPointAddress(x);
  // logProgramHeaderFileOffset(x);
  // logSectionHeaderFileOffset(x);
  // logSizeOfHeader(x);
  // logIndexOfStringTableInSectionHeaderTable(x);

  // ReadProgramHeaderTable(x, fd);
  // ReadSectionHeaderTable(x, fd);

  return 0;
}