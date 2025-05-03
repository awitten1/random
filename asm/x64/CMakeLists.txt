
cmake_minimum_required(VERSION 3.22)

project(x64_asm LANGUAGES CXX ASM)

enable_language(ASM)

set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(SOURCE some_func.s APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")

add_executable(
    main 
    main.cpp 
    some_func.asm
    #some_function.s
)