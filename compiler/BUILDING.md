# Building Spherepop

## Requirements

- C99 compiler (gcc, clang, or compatible)
- `make` or CMake ≥ 3.16
- `libm` (standard on all POSIX systems)

## Quick build (plain make)

```bash
make
./spherepop
```

## CMake build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

## Debug build with sanitizers

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DSP_SANITIZE=ON ..
make -j4
```

## Compiler warnings

All warnings in `cmake/CompilerWarnings.cmake` are enabled by default.
The codebase compiles clean at `-Wall -Wextra -Wpedantic`.

## Running tests

```bash
make test          # plain make
ctest              # CMake
```

## Cross-compilation

Set `CC` and adjust `CFLAGS` as needed. The codebase has no platform-
specific dependencies beyond POSIX libc and libm.
