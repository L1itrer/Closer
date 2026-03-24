#/bin/bash

set -xe

if [[ ! -d ./build ]]; then
  mkdir ./build
fi

CFLAGS="-Wall -Wextra -g -DCLOSER_DEBUG=1 -nostdlib -fno-builtin -fno-unwind-tables"

clang $CFLAGS ./src/main.c -o ./build/closer
