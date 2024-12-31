
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "dwarf.h"
#include "libdwarf.h"
#include <nlohmann/json.hpp>
//#include "duckdb.hpp"
#include <fmt/core.h>

#define PATH_MAX 256

// void CreateDBAndTable(duckdb::Connection& conn) {
//   auto res = conn.Query("ATTACH 'dwarf.db'; USE dwarf;");
//   if (res->HasError()) {
//     throw std::runtime_error{"attach"};
//   }
//   std::string ctas =
//   "CREATE TABLE IF NOT EXISTS dwarf ( "
//   " tag_name VARCHAR, "
//   " attributes JSON "
//   ")";
//   res = conn.Query(ctas);
//   if (res->HasError()) {
//     std::cout << res->GetError() << std::endl;
//     throw std::runtime_error{"failed ctas"};
//   }
// }

void assert_libdwarf_success(int res, Dwarf_Error error) {
  if (res == DW_DLV_ERROR) {
    char buf[100];
    sprintf(buf, "Error initializing DWARF: %s\n", dwarf_errmsg(error));
    throw std::runtime_error{buf};
  }
}

class Dwarf;

class DwarfAttribute {
  Dwarf_Attribute attr_;
  Dwarf_Debug dbg_;

  Dwarf_Half WhatAttr() {
    Dwarf_Half attrnum;
    Dwarf_Error error;

    int res = dwarf_whatattr(attr_, &attrnum, &error);
    assert_libdwarf_success(res, error);
    res = 0;
    return attrnum;
  }

  Dwarf_Half WhatForm() {
    Dwarf_Half form;
    Dwarf_Error error;
    int res = dwarf_whatform(attr_, &form, &error);
    assert_libdwarf_success(res, error);

    return form;
  }

public:

  struct DwarfOffset {
    Dwarf_Off off;
    Dwarf_Bool offIsInfo; // true if offset is an offset into .debug_info
    static std::string ToString(const DwarfOffset& off) {
      return fmt::format("offset = {}, offIsInfo = {}", off.off, off.offIsInfo);
    }
  };
  struct DwarfAddr {
    Dwarf_Addr addr;
  };
  struct DwarfExprLoc {
    Dwarf_Ptr loc;
    Dwarf_Unsigned length;
    static std::string ToString(const DwarfExprLoc& loc) {
      return fmt::format("loc = {}, length = {}", loc.loc, loc.length);
    }
  };
  using ValueType = std::variant<
    std::string,
    Dwarf_Signed,
    Dwarf_Unsigned,
    Dwarf_Bool,
    DwarfOffset,
    DwarfAddr,
    DwarfExprLoc,
    Dwarf_Block*
  >;

  DwarfAttribute(Dwarf_Attribute attr, Dwarf_Debug dbg) : attr_(attr), dbg_(dbg) {
  }

  std::string GetAttrName() {
    Dwarf_Half attrnum = WhatAttr();
    const char* name;
    dwarf_get_AT_name(attrnum, &name);
    return name;
  }

  std::string GetFormName() {
    Dwarf_Half form = WhatForm();
    const char* name;
    dwarf_get_FORM_name(form, &name);
    return name;
  }

  ~DwarfAttribute() {
    dwarf_dealloc_attribute(attr_);
  }

  ValueType GetValue() {
    Dwarf_Error error;
    int res;
    switch(WhatForm()) {
    case DW_FORM_string:
    case DW_FORM_line_strp:
    case DW_FORM_strp: {
      char *str_val;
      res = dwarf_formstring(attr_, &str_val, &error);
      assert_libdwarf_success(res, error);
      return ValueType(std::string(str_val));
    }

    case DW_FORM_ref4:
    case DW_FORM_sec_offset: {
      Dwarf_Off dw_return_offset;
      Dwarf_Bool dw_offset_is_info;
      res = dwarf_global_formref_b(attr_, &dw_return_offset, &dw_offset_is_info, &error);
      assert_libdwarf_success(res, error);
      return ValueType(DwarfOffset{dw_return_offset, dw_offset_is_info});
    }
    case DW_FORM_addr: {
      Dwarf_Addr addr;
      res = dwarf_formaddr(attr_, &addr, &error);
      assert_libdwarf_success(res, error);
      return ValueType(DwarfAddr{addr});
    }

    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_data4:
    case DW_FORM_data8:
    case DW_FORM_udata: {
      Dwarf_Unsigned uval;
      res = dwarf_formudata(attr_, &uval, &error);
      assert_libdwarf_success(res, error);
      return ValueType(uval);
    }

    case DW_FORM_exprloc: {
      Dwarf_Unsigned len;
      Dwarf_Ptr loc;

      res = dwarf_formexprloc(attr_, &len, &loc, &error);
      assert_libdwarf_success(res, error);
      return ValueType(DwarfExprLoc{loc, len});
    }

    case DW_FORM_implicit_const:
    case DW_FORM_sdata: {
      Dwarf_Signed sval;
      res = dwarf_formsdata(attr_, &sval, &error);
      assert_libdwarf_success(res, error);
      return ValueType(sval);
    }
    case DW_FORM_block1: {
      Dwarf_Block *block;
      res = dwarf_formblock(attr_, &block, &error);
      assert_libdwarf_success(res, error);

    }
    case DW_FORM_flag:
    case DW_FORM_flag_present: {
      Dwarf_Bool flag;
      res = dwarf_formflag(attr_, &flag, &error);
      assert_libdwarf_success(res, error);
      return ValueType(flag);
    }
    }
    std::cout << "form = " << WhatForm() << ", name = " << GetFormName() << std::endl;
    throw std::runtime_error{"unhandled dwarf FORM"};
  }

  static std::string ValueTypeToString(const ValueType& v) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
          return arg;
        else if constexpr (std::is_same_v<T, Dwarf_Unsigned>)
          return std::to_string(arg);
        else if constexpr (std::is_same_v<T, Dwarf_Signed>)
          return std::to_string(arg);
        else if constexpr (std::is_same_v<T, Dwarf_Bool>)
          return std::to_string(arg);
        else if constexpr (std::is_same_v<T, DwarfOffset>)
          return DwarfOffset::ToString(arg);
        else if constexpr (std::is_same_v<T, DwarfAddr>)
          return std::to_string(arg.addr);
        else if constexpr (std::is_same_v<T, DwarfExprLoc>)
          return DwarfExprLoc::ToString(arg);
        else
          throw std::runtime_error{"non-exhaustive visitor!"};
    }, v);
  }

};

class DwarfDie {

  Dwarf_Die die_;
  Dwarf_Debug dbg_;

public:
  DwarfDie(Dwarf_Die die, Dwarf_Debug dbg) : die_(die), dbg_(dbg) {}

  std::string GetTag() {
    Dwarf_Half tag;
    Dwarf_Error error;
    int res = dwarf_tag(die_, &tag, &error);
    assert_libdwarf_success(res, error);
    res = 0;

    const char* name;
    res = dwarf_get_TAG_name(tag, &name);
    assert_libdwarf_success(res, error);
    res = 0;
    return name;
  }

  void Log() {
    Dwarf_Attribute *attr_list;
    Dwarf_Signed attr_count;
    Dwarf_Error error;
    int res = dwarf_attrlist(die_, &attr_list, &attr_count, &error);
    assert_libdwarf_success(res, error);
    res = 0;

    nlohmann::json j;

    const char* name;
    for (int i = 0; i < attr_count; ++i) {


      DwarfAttribute attr(attr_list[i], dbg_);
      std::string name = attr.GetAttrName();
      std::string value = DwarfAttribute::ValueTypeToString(attr.GetValue());
      std::cout << "name = " << name << ", value = " << value << std::endl;
    }
  }
};

class Dwarf {

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

    DwarfDie d(die, dbg_);
    d.Log();

    char* name;
    res = dwarf_diename(die, &name, &error);
    assert_libdwarf_success(res, error);
    std::cout << "name = " << name << std::endl;

    res = dwarf_child(die, &child_die, &error);
    if (res == DW_DLV_NO_ENTRY) {
      return;
    }
    assert_libdwarf_success(res, error);

    recurse(child_die);

    Dwarf_Die curr_die = child_die;
    for (;;) {
      Dwarf_Die next_die;
      res = dwarf_siblingof_c(curr_die, &next_die, &error);
      assert_libdwarf_success(res, error);
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

  }

  Dwarf_Debug GetDbg() {
    return dbg_;
  }

private:
  Dwarf_Debug dbg_;
};





int main(int argc, char** argv) {
  std::string path = argv[1];

  // duckdb::DuckDB db;
  // duckdb::Connection conn(db);
  // CreateDBAndTable(conn);
  Dwarf dwarf(path);

  dwarf.getNext();

  return 0;
}