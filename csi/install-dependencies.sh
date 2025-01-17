
#!/bin/bash

set -eu

if [ $# != 1 ]
then
  echo "./install-dependencies.sh <install location>"
fi

INSTALL_DIR=$(realpath $1)

install_grpc() {
  [ -d grpc ] || \
    git clone --recurse-submodules -b v1.69.0 --depth 1 \
    --shallow-submodules https://github.com/grpc/grpc

  pushd grpc
  cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/grpc \
      -B build -G Ninja

  cmake --build build -j8
  cmake --install build
  popd
}

install_grpc