

set(PROTO_FILE "${CMAKE_CURRENT_LIST_DIR}/csi.proto")


set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${DEPS_ROOT}/grpc")
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)

set(GRPC_SOURCES "")
list(APPEND GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/csi.pb.cc")
list(APPEND GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/csi.pb.h")
list(APPEND GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/csi.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/csi.grpc.pb.h")

set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_CPP_PROTOBUF_PLUGIN_PROTOC $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

add_custom_command(
  OUTPUT ${GRPC_SOURCES}
  COMMAND "${_PROTOBUF_PROTOC}"
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${PROTO_DIR}"
    --plugin=protoc-gen-grpc="${_CPP_PROTOBUF_PLUGIN_PROTOC}"
    "${PROTO_FILE}"
  DEPENDS "${PROTO_FILE}")

add_library(csi_proto STATIC ${GRPC_SOURCES})
target_include_directories(csi_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


if (NOT TARGET protobuf::libprotobuf)
  message(FATAL_ERROR "protobuf::libprotobuf target does not exist")
endif()
target_link_libraries(csi_proto PUBLIC gRPC::grpc++ protobuf::libprotobuf)

add_library(server server.cpp)

target_include_directories(server PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(server PUBLIC csi_proto)

