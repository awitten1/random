#!/bin/bash

set -eux

dir=$(dirname $(realpath $0))

apt_installs() {
    sudo apt update && \
        sudo apt install -y cmake build-essential ninja-build
}

build_arrow() {

    mkdir -p $dir/deps/source
    mkdir -p $dir/deps/install
    mkdir -p $dir/deps/build
    (cd $dir/deps/source && [ ! -d arrow ] && \
        git clone --branch apache-arrow-20.0.0 https://github.com/apache/arrow || true )

    pushd $dir/deps


    cmake -S $dir/deps/source/arrow/cpp --preset ninja-release \
        -B $dir/deps/build/arrow -DCMAKE_INSTALL_PREFIX=$dir/deps/install -G Ninja

    cmake --build $dir/deps/build/arrow -j6
    cmake --install $dir/deps/build/arrow
    popd

}

apt_installs
build_arrow