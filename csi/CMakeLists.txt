

cmake_minimum_required(VERSION 3.25)

project(csi-cpp-template LANGUAGES CXX)

set(DEPS_ROOT "${CMAKE_CURRENT_LIST_DIR}/deps" CACHE STRING "Root of deps.")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)
