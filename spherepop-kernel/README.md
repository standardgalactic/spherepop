# spherepop-kernel

Reference Rust implementation of the Spherepop kernel as defined in
`Spherepop_Specification.tex`: the World model `(H, Ω)`, the four primitive
operators, the Arbiter, collapse rules, and an overlay manager — plus the
surface-calculus sugar (Link, Unlink, Choice, Merge, SetMeta) expressed
entirely in terms of the four primitives.

## Layout

- `src/event.rs` — the event alphabet `{Pop, Refuse, Bind, Collapse}` and
  the `Event` struct (ABI-shaped: stable discriminant + optional payload
  fields).
- `src/history.rs` — `History` (the free monoid over events), `State`,
  and `apply` (the pure per-event transition function whose fold is
  Replay). Includes `possibility_functional`, the conservation-law
  invariant `Π(H,Ω) = |Ω| + Σ w(e)`.
- `src/collapse.rs` — collapse rules `c : History → O_c`, pure functions
  external to `apply`. Includes `collapse_quotient`, which realizes
  Merge-sugar (`Merge_c(a,b) := Collapse_c(Bind(a,b))`), a refusal-honoring
  variant, `collapse_meta`, and the identity rule.
- `src/arbiter.rs` — `Arbiter`, the only path by which `History` is ever
  extended. Enforces: Pop only within Ω, Collapse only under a
  certified/registered rule, Refuse only with a non-empty reason.
  `validate` never receives a computed observation value `c(H)` for any
  rule — only structural `(H, Ω)` facts — which is how Observation
  Non-Interference is enforced by the type signature rather than by
  convention.
- `src/overlay.rs` — `OverlayManager`: preview a proposal against `H`
  without mutating it, then commit through the same `Arbiter::submit`
  path as any other proposal. No `auto_commit` method exists.
- `src/sugar.rs` — `link`, `unlink`, `choice`, `merge`, `set_meta`: each
  returns `Event`/`Vec<Event>` built only from the four primitives.
- `src/bin/trace.rs` — runnable worked example: introduce two objects,
  bind them, observe under a quotient rule (this *is* Merge), then
  withdraw reliance on the bind without deleting it.
- `src/bin/fixtures.rs` — conformance runner for the language-neutral
  flat fixture suite in `../experiments/flat/fixtures/`; see
  `../experiments/flat/README.md` for the fixture format. An independent
  Python oracle (`../experiments/flat/run_python.py`) reads the same
  fixtures, so agreement between the two is a small cross-implementation
  conformance result.
- `src/json.rs` — a minimal, dependency-free JSON reader/writer used only
  by `bin/fixtures.rs`, so the crate can read the fixture files without
  taking on `serde_json` as a dependency.

## Running

```sh
cargo test              # 20 unit tests
cargo run --bin trace
cargo run --bin fixtures  # flat fixture conformance suite
```

## What the tests check

Primitive-level behavior (Pop shrinks Ω; Refuse leaves Ω untouched; Bind
couples without identifying; Collapse requires a certified rule and
records only that an observation occurred, never its value), sugar
correctness (Link *is* Bind; Unlink leaves the original Bind in place;
Choice both commits and explicitly refuses; Merge is nothing but a
quotienting Collapse of a Bind; SetMeta is invisible to the ordinary
quotient rule), and kernel-level invariants (deterministic replay,
irreversibility, conservation of the possibility functional, overlay
preview never mutates `H`, stale-overlay commits are rejected, and a
later acceptance decision cannot depend on what an earlier Collapse
observed).

## Relationship to the specification

This crate is deliberately dependency-free (`std` only) so it compiles
offline and stays easy to audit against the LaTeX definitions line by
line. `EventKind` has exactly four variants, matching the specification's
claim that Sphere/Merge/Choice/Link/Unlink/Nest/SetMeta are all
second-order syntactic sugar rather than additional primitives.
