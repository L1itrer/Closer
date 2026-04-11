#/usr/bin/env bash

set -xe

if [[ ! -d ./build ]]; then
  mkdir ./build
fi

CC="gcc"

for arg in "$@"; do declare $arg='1'; done
if [ -v clang ]; then CC="clang"; fi

CFLAGS="-Wall -Wextra -nostdlib -fno-builtin -fno-unwind-tables -fno-stack-protector -static"

# if [ -v asm ]; then CFLAGS+=" -c -S"; fi
DEBUG_FLAGS="-g -DCLOSER_DEBUG=1 -DCLOSER_DISABLE=1"
RELEASE_FLAGS="-O2"

SPECIFIC_FLAGS=""
if [ -v release ]; then SPECIFIC_FLAGS=$RELEASE_FLAGS; fi
if [ ! -v release ]; then SPECIFIC_FLAGS=$DEBUG_FLAGS; fi

$CC $SPECIFIC_FLAGS $CFLAGS ./src/main.c -o ./build/closer
if [ -v asm ]; then $CC $SPECIFIC_FLAGS $CFLAGS ./src/main.c -o ./build/closer.asm -c -S; fi
# if [ ! -v release ]; then $CC $DEBUG_FLAGS $CFLAGS ./src/main.c -o ./build/closer; fi
