# Spherepop

**Geometry, Cognition, and the Transparency of Computation**

Spherepop is a programming language and runtime in which computation is
treated as a navigable history through constrained admissibility regions,
not a mapping from input to output. A bubble is a topological object —
a bounded region carrying scope, history, and constraint structure — not
a parenthesized subexpression. A pop event is a recorded, admissibility-
verified transformation, not a silent reduction step.

The monograph `spherepop-monograph.pdf` is the primary specification.
This implementation is its executable counterpart.

## Quick Start

```bash
# Build
make

# Run a program
./spherepop examples/arithmetic.sp

# Interactive REPL
./spherepop

# Evaluate an expression
./spherepop -e "print(2 ^ 10)"

# Show AST
./spherepop --ast examples/arithmetic.sp

# Show computation history
./spherepop --history examples/provenance.sp

# Trace all events
./spherepop --trace examples/bubble_nesting.sp
```

## Language Overview

```
// Variables
let x = 42;
let mut y = 10;

// Bubbles: explicit admissibility regions
let result = bubble {
    let inner = bubble { 3 * 4 };
    inner + 5
};

// Pop: explicit evaluation event with history recording
let val = pop(some_bubble);

// Refuse: mark inadmissible without erasing
fn safe_sqrt(x) {
    if (x < 0) { refuse(); return 0; }
    return sqrt(x);
}

// Functions
fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

// Observe a bubble's admissibility state
let report = observe(my_bubble);

// Inspect the global computation history
let h = history();
```

## Architecture

The implementation follows three irreducible design pivots:

1. **Graph allocation**: bubbles are heap-allocated nodes linked by
   parent/child pointers, not stack frames.

2. **Event dispatch**: `pop()` emits `EV_POP_BEGIN` and `EV_POP_COMMIT`
   events; provenance, admissibility, topology, and history subsystems
   are listeners. This prevents the evaluator from owning all subsystems
   sequentially and reintroducing the flattening problem.

3. **Native historical bubbles**: bubbles carry `History *`, `ConstraintSet *`,
   `ProvenanceID`, and admissibility scores as first-class fields — not
   external annotations on a conventional AST.

See `ARCHITECTURE.md` for the full design rationale.

## Directory Structure

```
src/runtime/    — Core: bubble, history, constraints, provenance, evaluator
src/lexer/      — Lexer and token types
src/parser/     — Recursive descent parser and AST
src/vm/         — Region VM (RegionInstruction bytecode)
src/geometry/   — Manifolds, curvature, boundary geometry
src/semantics/  — Sheaf semantics, observerhood, CLIO projection
src/stdlib/     — Standard library in Spherepop
examples/       — Annotated example programs
spec/           — Formal grammar and operational semantics
docs/           — Extended documentation
```

## Building with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSP_ENABLE_SANITIZERS=ON
cmake --build build
ctest --test-dir build
```

## Theoretical Background

The monograph develops the theoretical foundations across eight parts:

- **Part I**: Geometry of computation — nested containment, pop as
  explicit event, provenance preservation, topology.
- **Part II**: Cognitive and neuroscientific foundations.
- **Part III**: Thermodynamics — admissibility, entropy, action formalism.
- **Parts IV–VIII**: Emergence, philosophy of science (Feyerabend/Popper),
  ontology, geometry, and ethics of interpretability.
- **Appendices A–G**: Formal mathematics — history manifolds, variational
  semantics, thermodynamic admissibility, sheaf cohomology, observerhood,
  category-theoretic semantics, entropic geometry.
