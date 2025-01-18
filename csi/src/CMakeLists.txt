
add_subdirectory(grpc)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE server)
