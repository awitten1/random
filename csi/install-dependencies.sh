
#!/bin/bash

set -eu

DEPS_DIR=$(dirname $(realpath $0))/deps

install_grpc() {

  SRC_DIR=${DEPS_DIR}/src/grpc
  BULID_DIR=${DEPS_DIR}/build/grpc
  INSTALL_DIR=${DEPS_DIR}/install/grpc

  [ -d "${SRC_DIR}" ] || \
    git clone --recurse-submodules -b v1.69.0 --depth 1 \
    --shallow-submodules https://github.com/grpc/grpc "${SRC_DIR}"

  cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
      -B "${BULID_DIR}" -G Ninja \
      -S "${SRC_DIR}"

  cmake --build "${BULID_DIR}" -j8
  cmake --install "${BULID_DIR}"
}

setup_directories() {
  mkdir -p ${DEPS_DIR}
  mkdir -p ${DEPS_DIR}/install
  mkdir -p ${DEPS_DIR}/src
  mkdir -p ${DEPS_DIR}/build
}

setup_directories
install_grpc