cmake_minimum_required(VERSION 3.20)
project(learn_arm LANGUAGES C CXX ASM)
# ^ Listing ASM here is the whole trick: CMake then knows how to drive the
#   assembler for .S files, using the C compiler as the driver. A capital-S
#   .S means "run the C preprocessor first" (so #include and #if work);
#   a lowercase .s would skip preprocessing.

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  message(FATAL_ERROR
    "This project is hand-written AArch64 assembly; "
    "CMAKE_SYSTEM_PROCESSOR is '${CMAKE_SYSTEM_PROCESSOR}'.")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------------
# The assembly. Drop a new .S file into src/asm/ and it gets picked up on the
# next build (CONFIGURE_DEPENDS re-runs the glob when the directory changes).
# --------------------------------------------------------------------------
file(GLOB ASM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/asm/*.S")
if(NOT ASM_SOURCES)
  message(FATAL_ERROR "No .S files found in src/asm/")
endif()

add_library(asm_kernels STATIC ${ASM_SOURCES})
target_include_directories(asm_kernels PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
# -g keeps the .S line numbers in the debug info, so lldb can step through the
# assembly source rather than raw addresses.
target_compile_options(asm_kernels PRIVATE -g)

# --------------------------------------------------------------------------
# C reference implementations, built optimised so the comparison is honest.
# --------------------------------------------------------------------------
add_library(c_kernels STATIC src/c_kernels.c)
target_include_directories(c_kernels PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(c_kernels PRIVATE -O3)

add_library(kernels INTERFACE)
target_link_libraries(kernels INTERFACE asm_kernels c_kernels)

# --------------------------------------------------------------------------
# Scratch pad, tests, benchmarks.
# --------------------------------------------------------------------------
add_executable(playground src/playground.c)
target_link_libraries(playground PRIVATE kernels)

enable_testing()
add_executable(test_kernels tests/test_kernels.c)
target_link_libraries(test_kernels PRIVATE kernels)
add_test(NAME kernels COMMAND test_kernels)

find_package(benchmark REQUIRED)
add_executable(bench_kernels bench/bench_kernels.cc)
target_link_libraries(bench_kernels PRIVATE kernels benchmark::benchmark)

# --------------------------------------------------------------------------
# Learning aids.
#
#   cmake --build build --target disasm    what the assembler made of your .S
#   cmake --build build --target cref-asm  what the compiler writes for the C
# --------------------------------------------------------------------------
find_program(OBJDUMP NAMES llvm-objdump objdump)
if(OBJDUMP)
  add_custom_target(disasm
    COMMAND "${OBJDUMP}" -d --no-show-raw-insn $<TARGET_FILE:asm_kernels>
    DEPENDS asm_kernels
    COMMENT "Disassembling asm_kernels"
    VERBATIM USES_TERMINAL)
endif()

add_custom_target(cref-asm
  COMMAND "${CMAKE_C_COMPILER}" -O3 -S -fverbose-asm
          -I"${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/src/c_kernels.c"
          -o "${CMAKE_CURRENT_BINARY_DIR}/c_kernels.s"
  COMMAND "${CMAKE_COMMAND}" -E echo "Wrote ${CMAKE_CURRENT_BINARY_DIR}/c_kernels.s"
  COMMENT "Compiling the C reference to assembly for comparison"
  VERBATIM USES_TERMINAL)
