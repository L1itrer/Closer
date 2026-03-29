#/usr/bin/env bash

set -xe

if [[ ! -d ./build ]]; then
  mkdir ./build
fi

CC="gcc"

for arg in "$@"; do declare $arg='1'; done
if [ -v clang ]; then CC="clang"; fi

CFLAGS="-Wall -Wextra -nostdlib -fno-builtin -fno-unwind-tables"
DEBUG_FLAGS="-g -DCLOSER_DEBUG=1 -DCLOSER_DISABLE=1"
RELEASE_FLAGS="-O2"

if [ -v release ]; then $CC $RELEASE_FLAGS $CFLAGS ./src/main.c -o ./build/closer; fi
if [ ! -v release ]; then $CC $DEBUG_FLAGS $CFLAGS ./src/main.c -o ./build/closer; fi
