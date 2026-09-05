#!/bin/sh
# One-command conformance runner: executes the flat fixture suite against
# every available conforming implementation and fails if any disagree with
# their own expectations. Currently wired up for:
#   - the Rust reference kernel (spherepop-kernel, via `cargo run --bin fixtures`)
#   - an independent Python oracle (run_python.py)
#   - the independent Go implementation (spherepop-go/cmd/fixtures)
#   - a standalone C port of the canonical model (compiler/tools/fixtures/,
#     built as the `sp_fixtures` CMake target; independent of compiler/'s
#     general-purpose Bubble interpreter -- see compiler/tools/fixtures/kernel.h)
#
# Usage: ./run_all.sh
set -eu

cd "$(dirname "$0")"

status=0

echo "== Python oracle =="
if python3 run_python.py; then
  echo "Python oracle: ALL PASS"
else
  echo "Python oracle: FAILURES"
  status=1
fi

echo
echo "== Rust reference kernel (spherepop-kernel) =="
if (cd ../../spherepop-kernel && cargo run --quiet --bin fixtures); then
  echo "Rust kernel: ALL PASS"
else
  echo "Rust kernel: FAILURES"
  status=1
fi

echo
echo "== Go implementation (spherepop-go) =="
if (cd ../../spherepop-go && go test ./... && go run ./cmd/fixtures ../experiments/flat/fixtures); then
  echo "Go implementation: ALL PASS"
else
  echo "Go implementation: FAILURES (or no Go toolchain available -- see CONFORMANCE.md)"
  status=1
fi

echo
echo "== C kernel (compiler/tools/fixtures) =="
if (
  mkdir -p ../../compiler/build
  cd ../../compiler/build
  cmake .. >/dev/null
  make sp_fixtures >/dev/null
  ./sp_fixtures
); then
  echo "C kernel: ALL PASS"
else
  echo "C kernel: FAILURES (or no C toolchain available -- see CONFORMANCE.md)"
  status=1
fi

echo
echo "== Generating conformance matrix (CONFORMANCE.md) =="
python3 generate_conformance_matrix.py || status=1

exit $status
