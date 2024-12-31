cmake_minimum_required(VERSION 3.22)

project(dwarf-tools CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dwarf-tool main.cc)
target_compile_options(dwarf-tool PRIVATE "-g")

include(FindPkgConfig)

pkg_check_modules(LIBDWARF REQUIRED libdwarf)

target_include_directories(dwarf-tool PUBLIC ${LIBDWARF_INCLUDE_DIRS})
target_link_libraries(dwarf-tool PUBLIC ${LIBDWARF_LINK_LIBRARIES})

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(dwarf-tool PRIVATE nlohmann_json::nlohmann_json)


if(NOT DUCKDB_ROOT)
    message(FATAL_ERROR "Must provide DUCKDB_ROOT variable must be set.")
endif()
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${DUCKDB_ROOT})

# find_library(DUCKDB_LIB NAMES duckdb REQUIRED)
# find_file(DUCKDB_HEADER NAMES duckdb.hpp REQUIRED)
# target_include_directories(dwarf-tool PRIVATE ${DUCKDB_ROOT})
# target_link_libraries(dwarf-tool PRIVATE ${DUCKDB_LIB})

find_package(fmt REQUIRED)
target_link_libraries(dwarf-tool PRIVATE fmt::fmt)