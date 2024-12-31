#!/bin/bash


deps=(
  build-essential
  xz-utils
  pkg-config
  zlib1g
  zlib1g-dev
  libzstd1
  cmake
  curl
  ninja-build
  unzip
)

install_libdwarf() {
  DIR=$PWD

  LIBDWARF_VERSION=0.11.1
  dwarf_path="https://github.com/davea42/libdwarf-code/releases/download/v${LIBDWARF_VERSION}/libdwarf-${LIBDWARF_VERSION}.tar.xz"
  echo "Curling URL $dwarf_path"
  curl -L  $dwarf_path -o $DIR/libdwarf-${LIBDWARF_VERSION}.tar.xz
  tar -xf $DIR/libdwarf-${LIBDWARF_VERSION}.tar.xz
  rm -rf /tmp/build
  mkdir /tmp/build
  pushd /tmp/build
  $DIR/libdwarf-${LIBDWARF_VERSION}/configure
  make
  make check
  make install
  popd
}

install_duckdb() {
  arch=$(uname -m)
  duckdb_version="v1.1.3"
  url="https://github.com/duckdb/duckdb/releases/download/${duckdb_version}/libduckdb-linux-${arch}.zip"
  mkdir -p /tmp/duckdb

  curl -L $url -o /tmp/duckdb/libduckdb.zip
  pushd /tmp/duckdb
  unzip /tmp/duckdb/libduckdb.zip
  popd
}

install_deps() {
  apt update
  apt install -y ${deps[@]}
}

install_deps
install_libdwarf
install_duckdb
