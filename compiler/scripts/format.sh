#!/bin/sh
find src -name '*.c' -o -name '*.h' | xargs clang-format -i
echo "Format complete."
