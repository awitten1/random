#!/bin/bash


deps=(
  build-essential
  xz-utils
  pkg-config
  zlib1g
  zlib1g-dev
  libzstd1
  cmake
)

install_libdwarf() {
  DIR=$PWD

  LIBDWARF_VERSION=0.11.1

  curl -L "https://github.com/davea42/libdwarf-code/releases/download/v${libd_v}/libdwarf-${libd_v}.tar.xz" \
    -o $DIR/libdwarf-${LIBDWARF_VERSION}.tar.xz
  tar -xf $DIR/libdwarf-${LIBDWARF_VERSION}.tar.xz
  rm -rf /tmp/build
  mkdir /tmp/build
  cd /tmp/build
  $DIR/libdwarf-${LIBDWARF_VERSION}/configure
  make
  make check
  make install
}

install_deps() {
  apt update
  apt install -y ${deps[@]}
}

install_libdwarf
install_deps