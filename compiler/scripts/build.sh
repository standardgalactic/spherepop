#!/bin/sh
set -e
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc 2>/dev/null || echo 4)"
echo "Build complete: build/spherepop"
