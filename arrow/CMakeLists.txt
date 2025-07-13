cmake_minimum_required(VERSION 3.22)

project(arrow-learn LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(main main.cpp)

include_directories(perfcxx)

find_package(Arrow REQUIRED)
target_link_libraries(main PRIVATE Arrow::arrow_shared)