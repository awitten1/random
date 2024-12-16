
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include "dwarf.h"
#include "libdwarf.h"

#define PATH_MAX 256

class Dwarf {

  void ThrowOnError(int error) {
    if (error == DW_DLV_ERROR) {
      throw std::runtime_error{"failed operation"};
    }
  }

public:
  Dwarf(const std::string& path) {
    Dwarf_Debug dbg = 0;
    char true_path[PATH_MAX + 1];
    unsigned int true_pathlen = 0;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    Dwarf_Error error = 0;

    int res = dwarf_init_path(path.c_str(),true_path,sizeof(true_path),
      DW_GROUPNUMBER_ANY,errhand,errarg,&dbg,&error);

    if (res == DW_DLV_OK) {
      std::cout << "success opening " << path << std::endl;
    } else if (res == DW_DLV_NO_ENTRY) {
      fprintf(stderr, "File not found or no DWARF information available.\n");
    } else { // DW_DLV_ERROR
      fprintf(stderr, "Error initializing DWARF: %s\n", dwarf_errmsg(error));
    }

    dbg_ = dbg;
  }

  void recurse(Dwarf_Die die) {
    int res;
    Dwarf_Error error;
    Dwarf_Die child_die = 0;

    char* name;
    res = dwarf_diename(die, &name, &error);
    ThrowOnError(res);
    std::cout << "name = " << name << std::endl;

    res = dwarf_child(die, &child_die, &error);
    if (res == DW_DLV_NO_ENTRY) {
      return;
    }
    ThrowOnError(res);

    recurse(child_die);

    Dwarf_Die curr_die = child_die;
    for (;;) {
      Dwarf_Die next_die;
      res = dwarf_siblingof_c(curr_die, &next_die, &error);
      ThrowOnError(res);
      if (res == DW_DLV_NO_ENTRY) {
        return;
      }
      recurse(next_die);
      curr_die = next_die;
    }
  }

  void getNext() {
    Dwarf_Error error = 0;
    Dwarf_Die die = 0;
    Dwarf_Unsigned dw_cu_header_length;
    Dwarf_Half     dw_version_stamp;
    Dwarf_Off      dw_abbrev_offset;
    Dwarf_Half     dw_address_size;
    Dwarf_Half     dw_length_size;
    Dwarf_Half     dw_extension_size;
    Dwarf_Sig8     dw_type_signature;
    Dwarf_Unsigned dw_typeoffset;
    Dwarf_Unsigned dw_next_cu_header_offset;
    Dwarf_Half     dw_header_cu_type;

    for (;;) {
      int res = dwarf_next_cu_header_e(dbg_,
        true, &die, &dw_cu_header_length, &dw_version_stamp, &dw_abbrev_offset,
        &dw_address_size, &dw_length_size, &dw_extension_size, &dw_type_signature,
        &dw_typeoffset, &dw_next_cu_header_offset, &dw_header_cu_type, &error);
      if (res == DW_DLV_OK) {
        fprintf(stderr, "dwarf_next_cu_header_e was successful\n");
      } else if (res == DW_DLV_NO_ENTRY) {
        fprintf(stderr, "No more CU.\n");
        break;
      } else { // DW_DLV_ERROR
        fprintf(stderr, "Failed getting next CU: %s\n", dwarf_errmsg(error));
        break;
      }
      error = 0;
      char* dw_name;
      res = dwarf_diename(die, &dw_name, &error);
      std::cout << "dw_name = " << dw_name << std::endl;
      recurse(die);
    }

    dwarf_srcfiles;
  }
private:
  Dwarf_Debug dbg_;
};





int main(int argc, char** argv) {
  std::string path = argv[1];
  Dwarf dwarf(path);

  dwarf.getNext();

  return 0;
}