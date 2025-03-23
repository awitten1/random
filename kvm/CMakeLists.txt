cmake_minimum_required(VERSION 3.24)

project(kvm_random LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(kvm_main kvm_main.cpp)
add_library(kvm kvm_vm.cpp)
target_include_directories(kvm PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(kvm_main PUBLIC kvm)