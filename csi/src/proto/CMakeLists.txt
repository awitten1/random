

set(PROTO_FILE csi.proto)


set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${DEPS_ROOT}/grpc")
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_library(csi_proto)
protobuf_generate(TARGET csi_proto LANGUAGE cpp PROTOS ${PROTO_FILE})

if (NOT TARGET protobuf::libprotobuf)
  message(FATAL_ERROR "protobuf::libprotobuf target does not exist")
endif()
target_link_libraries(csi_proto PUBLIC grpc++ protobuf::libprotobuf)

