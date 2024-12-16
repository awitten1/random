cmake_minimum_required(VERSION 3.22)

project(dwarf-tools CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dwarf-tool main.cc)
target_compile_options(dwarf-tool PRIVATE "-g")

include(FindPkgConfig)

pkg_check_modules(LIBDWARF REQUIRED libdwarf)

target_include_directories(dwarf-tool PUBLIC ${LIBDWARF_INCLUDE_DIRS})
target_link_libraries(dwarf-tool PUBLIC ${LIBDWARF_LINK_LIBRARIES})
