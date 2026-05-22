# Spherepop Architecture

## Three Irreducible Design Pivots

### 1. Graph Allocation

Bubbles are heap-allocated nodes linked by parent/child pointers.
`bubble_alloc(parent, scope)` is the primary constructor. The region
allocator in `src/runtime/memory.c` groups allocations by bubble
lifetime: a bubble and all its descendants share a region that is
released at pop.

This is the correct memory model because nested bubble lifetimes
are hierarchical and predictable — exactly what region allocators
are designed for.

### 2. Event Dispatch

`pop()` does **not** call provenance, admissibility, topology, or
history subsystems directly. It emits `EV_POP_BEGIN` and
`EV_POP_COMMIT` events via `event_emit(ctx->bus, &ev)`. Subsystems
register as listeners via `event_subscribe()`.

This prevents the evaluator from becoming a recursive owner of all
subsystems, which would silently reintroduce the flattening problem
that Spherepop exists to critique. The event bus supports lazy
listener evaluation: listeners receive the event reference and may
defer computation until their output is queried (demand-driven
provenance — not everything that happens needs to be fully materialized).

Standard listeners registered by default:
- `admissibility_listener` — on `EV_POP_BEGIN`
- `provenance_listener`    — on `EV_POP_COMMIT`
- `history_listener`       — on `EV_POP_COMMIT`
- `topology_listener`      — on `EV_POP_COMMIT`

### 3. Native Historical Bubbles

The `Bubble` struct carries:
- `History *history`           — local pop event sequence
- `ConstraintSet *constraints` — relational constraint graph
- `uint64_t prov_id`           — back-reference into global provenance
- `double local_admissibility` — [0,1] constraint satisfaction score
- `double action_cost`         — accumulated S[γ] = Σ L_t

Bubbles are **not** erased at pop. They are transformed from
`BUBBLE_ACTIVE` to `BUBBLE_POPPED` and their pop event is recorded
in `ctx->global_history`. The structural information they encoded —
scope, constraints, evaluation history — is preserved, not consumed.

## Constraint Design

`ConstraintSet` uses a constraint graph rather than a bitmask.
`ConstraintRef` in `RegionInstruction` is a table index, not an
inline bitmask. This is because constraints are relational and
directional — they compose, interact, and have directed propagation.
A flat bitmask breaks at the first constraint interaction.

## Admissibility Returns AdmissibilityPoint, Not double

`observer_evaluate(obs, history)` returns `AdmissibilityPoint` —
a position in a structured space — not a raw `double`. A scalar
collapses the admissibility geometry immediately. This is the same
failure mode the monograph documents in conventional runtimes; the
implementation must not reproduce it in its own observer interface.

## Dependency Strata

1. **Pure syntactic**: lexer, parser, formatter, AST transforms
2. **Runtime-semantic**: bubble, history, collapse, admissibility — these
   are the core; `admissibility_checker` depends on this stratum, not
   on external tooling
3. **Observational/tooling**: visualizers, provenance browsers, docgen

Tools that wrap runtime subsystems belong in stratum 2's dependency
graph, not in a `tools/` directory as though they were external utilities.

## VM Design

The VM operates on `RegionInstruction`s, not stack instructions.
`PUSH 3; PUSH 4; MUL` destroys topological semantics immediately.
Each `RegionInstruction` carries `RegionID`, `ConstraintRef`,
`ProvenanceRef`, and `OpCode`. Bubbles are first-class VM objects:
`OP_ENTER_REGION`/`OP_EXIT_REGION`/`OP_POP_BUBBLE`/`OP_REFUSE` are
distinct opcodes, not call-convention sugar.

The tree-walking evaluator in `src/runtime/evaluator.c` is primary.
The VM in `src/vm/` is a future hot-path backend; the compilation
boundary must not erase bubble topology.
