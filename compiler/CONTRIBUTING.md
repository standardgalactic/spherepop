# Contributing to Spherepop

## Principle of Philosophical Alignment

Before adding a feature, check that it does not contradict the monograph.
Specifically:
- Does your change preserve bubble provenance through the relevant operation?
- Does your change treat computation as history, not as state mutation?
- Does your change add a subsystem that calls into the evaluator directly
  rather than subscribing to events? (If yes: rethink the dependency direction.)

## Build

```bash
make spherepop
./spherepop examples/arithmetic.sp
```

## Tests

```bash
make test
```

Add integration tests as `.sp` files in `tests/integration/`.
Add unit tests in `tests/runtime/`, `tests/lexer/`, etc.

## Literate Extraction

Theorem environments in the monograph `.tex` source should eventually
generate executable tests. If you find a case where a monograph theorem
fails as a runtime invariant, file a `semantic-regression` issue.

## Code Style

- C99, no extensions.
- `clang-format` with the project's `.clang-format`.
- Every public header has a comment explaining its relationship to the
  monograph. If it does not, add one before submitting.
- No function in `evaluator.c` should call a subsystem directly if an
  event could carry the information instead.
