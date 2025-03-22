cmake_minimum_required(VERSION 3.24)

project(kvm_random LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(kvm_main kvm_main.cpp)