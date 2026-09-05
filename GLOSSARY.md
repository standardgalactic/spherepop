# Spherepop vocabulary glossary

This is the Phase A / Section 6 deliverable from issue #1: one entry per
recurring term, each assigned exactly one **layer** (so ontology,
semantics, syntax, implementation, and visualization are never
conflated) and one **status**:

- **canonical** — defined by the current normative specification. As of
  the Phase B deliverable, the four primitive events (`Pop`, `Refuse`,
  `Bind`, `Collapse`), the world model, and their derived forms are
  frozen in `SPEC.md`, which is the authoritative source for those
  terms and supersedes any conflicting definition below; the broader
  monograph (`Spherepop_Specifications.tex`) remains the source for
  everything outside that four-primitive core. Both are executed
  literally by `spherepop-kernel` (see `IMPLEMENTATIONS.md`).
- **derived** — defined *in terms of* the canonical primitives (sugar),
  per the specification's completeness argument.
- **analogy** — an illustrative claim relating Spherepop concepts to
  something outside the calculus (types, proofs, programs); not itself
  a definition to be implemented.
- **experimental** — used by an implementation still working toward
  conformance; meaning may not yet match the canonical definition.
- **historical** — used with a real, different meaning by an artifact
  that predates the four-primitive consolidation. Kept as evidence of
  development, not treated as a deviation to be "fixed."
- **false friend** — same *word* as a canonical term, different
  *meaning*, in an implementation that does not claim conformance. Not
  wrong on its own terms, but a documented trap for readers moving
  between implementations.
- **unresolved** — named and defined in the specification's prose but
  not yet backed by an executable definition anywhere in the repo.

Every local synonym below cites where it is used and, if it diverges,
what it means there instead — per the tracking issue, an unmarked
semantic divergence is not acceptable, but a marked one is exactly the
point of this document.

## 1. Semantic entities

| Term | Layer | Status | Definition | Citations |
|---|---|---|---|---|
| History (`H`) | semantic entity | canonical | The free monoid over the event alphabet `E = {Pop, Refuse, Bind, Collapse}`: append-only, no remove/undo. | `Spherepop_Specifications.tex:156`; `spherepop-kernel/src/history.rs` (`History`, `push`, no mutator but append) |
| Event | semantic entity | canonical | One element of the fixed alphabet `E`; every field beyond kind/position is optional and kind-dependent. | `spherepop-kernel/src/event.rs` |
| Option / Object (`ObjectId`) | semantic entity | canonical | A member of the initial option space `Ω₀`; may be committed, refused, bound, or left untouched. | `spherepop-kernel/src/event.rs` (`ObjectId`) |
| Option space (`Ω`) | semantic entity | canonical | The current subset of `Ω₀` that remains possible; shrinks only via Pop. | `Spherepop_Specifications.tex:156`; `spherepop-kernel/src/history.rs` (`State.option_space`) |
| Object identity | semantic entity | canonical | Two objects are never made identical by Bind; identity is only ever produced observationally, by a Collapse rule that chooses to quotient them (see Merge). | `spherepop-kernel/src/collapse.rs` (`UnionFind`, `collapse_quotient`) |
| Dependency | semantic entity | canonical | The relation recorded by Bind between two objects; observable to collapse rules, does not itself identify them. | `spherepop-kernel/src/event.rs` (`Event::bind`) |
| Refusal record | semantic entity | canonical | `(position, target, reason)` recorded by Refuse; `Ω` is untouched by it. | `spherepop-kernel/src/history.rs` (`State.refused`) |
| Rule (collapse rule, `c`) | semantic entity | canonical | A pure function `History -> O_c`, external to `apply`, certified/registered with the Arbiter before use. | `spherepop-kernel/src/collapse.rs`; `spherepop-kernel/src/arbiter.rs` (`rules`) |
| Quotient | semantic entity | canonical | The equivalence-class structure a given collapse rule imposes on bound objects (e.g. union-find classes under `collapse_quotient`). | `spherepop-kernel/src/collapse.rs` (`UnionFind::same_class`) |
| Observation | semantic entity | canonical | The value `c(H)` produced by a Collapse rule; never itself stored in `State`, and never visible to the Arbiter's acceptance decision (Observation Non-Interference). | `spherepop-kernel/src/arbiter.rs` (module doc); `spherepop-kernel/src/history.rs` (`State.observed` stores only `(pos, rule)`, not a value) |

## 2. Primitive events

| Term | Layer | Status | Canonical meaning | Local synonyms / deviations |
|---|---|---|---|---|
| **Pop** | primitive event | canonical | Commit to a specific option, removing it from what remains possible. | **false friend** in `reduction-engine/spherepop.py`: an outcome label meaning "this instruction reduced successfully" (no relation to an option space). **historical**, unrelated meaning ("function application"/term elimination) in `spherepop.hs`/`spherepop.rkt` and in the spec's own embedded SPC term calculus (`Spherepop_Specifications.tex:484`, "$\Pop$ eliminates [a Π-type]" — itself explicitly marked in the spec as a *different, term-calculus* sense of Pop, distinct from the primitive commitment operator, `Spherepop_Specifications.tex:281`). **experimental** in `compiler/`: a bubble lifecycle event around evaluation (`EV_POP_BEGIN`/`EV_POP_COMMIT`), closely aligned but scoped to one bubble rather than a general `Ω`. **experimental**, close to canonical, in `prototypes/sphereforth_gforth.zip`'s `POP` word (irreversibly drop the top option, log it). |
| **Refuse** | primitive event | canonical | Record an option as inadmissible without removing it from what remains possible. | **experimental**, close to canonical, in `compiler/` (`EV_REFUSE`, marks a bubble `BUBBLE_REFUSED`) and in SphereForth's `REFUSE` word ("same stack effect as POP, different accounting label" — its own README explicitly distinguishes the two). Loosely related but narrower in `reduction-engine/spherepop.py`: a per-instruction admissibility-gate rejection outcome, not a standalone event over an option space. Absent entirely from `spherepop.hs`/`spherepop.rkt` and the Python `Region` prototype. |
| **Bind** | primitive event | canonical | Couple two elements as dependent without identifying them. | **experimental, deviating** in `compiler/`: implemented as constraint-set merge onto one bubble's state (`cset_merge` + state transition) — closer to "attach constraints to this scope" than "couple two elements." **experimental, close to canonical** in SphereForth's `BIND` word (logs a dependency, keeps both options on the stack). Absent from `reduction-engine/spherepop.py`, `spherepop.hs`, `spherepop.rkt`, and the Python `Region` prototype. |
| **Collapse** | primitive event | canonical | Observe a history under a chosen rule by projecting it onto that rule's quotient space; never mutates state. | **experimental, incomplete** in `compiler/`: `bubble_collapse()` only sets a state flag and logs the event — the quotient-space semantics described in its own `collapse.h` are not yet wired into the evaluator. **experimental, close to canonical** in SphereForth's `COLLAPSE` word (replaces two options with a synthesized quotient token, logs the quotient). **false friend** in `reduction-engine/spherepop.py`: means "commit a value upward to the parent bubble" (a return-value mechanism), not an observation over history. |

## 3. Structural composition

| Term | Layer | Status | Definition | Citations |
|---|---|---|---|---|
| Meld | structural composition | canonical | Parallel composition (free monoidal tensor) of two independently-generated histories; a true structural operation on the history monoid, but not required by the concurrency clause of the Completeness Theorem (that goes through Bind alone). Not a primitive event. | `spherepop-kernel/src/history.rs` (`History::meld`); `experiments/flat/fixtures/08_meld.json` (marked `manual: true` — not yet exercised by the single-Arbiter flat-fixture schema; see that fixture's `explanation` field) |

## 4. Derived surface forms

All entries below are **derived**: each must expand into only `Pop`/
`Refuse`/`Bind`/`Collapse` events, per the specification's completeness
argument (`Spherepop_Specifications.tex:153`). None is its own
`EventKind` variant in the normative reference.

| Term | Canonical definition | Executable status |
|---|---|---|
| Sphere | `Sphere(x:A.t) := Bind(x, Ω)` — abstraction freezes/scopes the option space relative to `x` by binding, not by a separate lambda-specific event. | **unresolved**: defined in `Spherepop_Specifications.tex:330` but not implemented in `spherepop-kernel/src/sugar.rs` (only `link`, `unlink`, `choice`, `merge`, `set_meta` exist there). |
| Merge | `Merge_c(a,b) := Collapse_c(Bind(a,b))` — a Bind followed by an observation under an identifying quotient rule. | **canonical, implemented**: `spherepop-kernel/src/sugar.rs` (`merge`), exercised by `experiments/flat/fixtures/06_derived_merge.json`. |
| Choice | `Choice(taken, rejected) := Pop(taken)` alongside an explicit `Refuse(rejected)`, rather than letting the untaken branch silently vanish. | **canonical, implemented**: `spherepop-kernel/src/sugar.rs` (`choice`), exercised by `experiments/flat/fixtures/12_small_abstraction.json` and `07_desugaring.json`. |
| Link | `Link(a,b) := Bind(a,b)`, exactly. | **canonical, implemented**: `spherepop-kernel/src/sugar.rs` (`link`). |
| Unlink | `Unlink(a,b) := Refuse(Bind(a,b))` — documents withdrawal of reliance on a Bind without deleting the original Bind event (irreversibility). | **canonical, implemented**: `spherepop-kernel/src/sugar.rs` (`unlink`), exercised by `experiments/flat/fixtures/07_desugaring.json`. |
| Nest | Sugar for a chain of `Bind`/`Collapse_{c_subst}` pairs (nested abstraction/application); described in the DSL surface table as "nested Pop." | **unresolved**: defined in `Spherepop_Specifications.tex:362-363` but not implemented in `spherepop-kernel/src/sugar.rs`. |
| SetMeta | `SetMeta(o,k,v) := Bind(o,(k,v))` under a distinguished metadata rule every other collapse rule is defined to ignore. | **canonical, implemented**: `spherepop-kernel/src/sugar.rs` (`set_meta`), `spherepop-kernel/src/collapse.rs` (`collapse_meta`), exercised by `experiments/flat/fixtures/07_desugaring.json`. |
| Burst | Surface DSL sugar for "nested Pop" (`burst g(a,b,...)`). | **unresolved**: named in `Spherepop_Specifications.tex:230,266` (grammar/DSL table) but has no corresponding function in `spherepop-kernel/src/sugar.rs`. |
| Stack words (`OPT`, `.LOG`, `REPLAY`, `.STACK`) | Not part of the canonical surface calculus; SphereForth-specific tooling built around its own log format. | **experimental**, local to `prototypes/sphereforth_gforth.zip` only. |

## 5. Implementation mechanisms

| Term | Layer | Status | Definition | Citations |
|---|---|---|---|---|
| Arbiter | implementation mechanism | canonical | The sole path by which `H` is ever extended; validates a proposal's events against `(H, Ω)` only — never against any collapse rule's observed value (Observation Non-Interference is enforced by the type signature of `validate`, not by convention). | `Spherepop_Specifications.tex:447-456`; `spherepop-kernel/src/arbiter.rs` |
| Overlay (manager) | implementation mechanism | canonical | Creates, previews, or discards a proposal held outside `H` without mutating it; commitment requires a separate, explicit call routed back through the Arbiter — there is deliberately no `auto_commit` path. | `Spherepop_Specifications.tex:470,475`; `spherepop-kernel/src/overlay.rs` |
| Frontier | implementation mechanism | experimental | SphereForth's name for the live stack of not-yet-popped option tokens, reconstructed by `REPLAY`. Not a term used by the canonical spec or the Rust kernel (which uses `Ω`/`option_space` instead). | `prototypes/sphereforth_gforth.zip` (`README.md` inside the archive: "a stack-based 'Ω-frontier'") |
| Parser | implementation mechanism | canonical (per-implementation) | Decodes a concrete syntax into events or an AST; every implementation has its own, unrelated in structure to any other. | `compiler/src/lexer/`, `compiler/src/parser.c`; `prototypes/python/spherepop/parser.py` |
| Evaluator | implementation mechanism | canonical (per-implementation) | The component that actually executes/replays events or reduces terms; canonical only in `spherepop-kernel` (`History::replay` + `apply`) — other implementations' evaluators use their own (sometimes diverging) semantics, see §2 above. | `spherepop-kernel/src/history.rs` (`apply`, `replay`); `compiler/src/runtime/evaluator.c` |
| VM / region VM | implementation mechanism | experimental | Planned bytecode/region execution backend for `compiler/`; per its own `ROADMAP.md`, not yet implemented (Phase 2+, unchecked). | `compiler/ROADMAP.md` |
| Event bus | implementation mechanism | experimental | `compiler/`'s internal dispatch mechanism for lifecycle events (`EV_POP_BEGIN`, `EV_SCOPE_EXIT`, `EV_ADMISSIBILITY_RECALC`, etc.), broader than the canonical four-event alphabet. | `compiler/src/runtime/evaluator.h` |
| Serializer / canonical output | implementation mechanism | unresolved | No canonical wire format or digest function is specified yet (Phase B checklist item "Specify serialization and canonical output" is open); `experiments/flat`'s `09_replay.json` currently only checks in-memory replay determinism, not a serialize/deserialize round trip. | `experiments/flat/README.md` ("Known gaps" section) |

## 6. Visual vocabulary

These terms describe the *interactive/visual* layer only. None of them
denotes an executable primitive; where a demo's code was checked
directly (see `IMPLEMENTATIONS.md`), it had no corresponding method
(`pop()`, `bind()`, `collapse()`) implementing the calculus.

| Term | Layer | Status | Definition | Citations |
|---|---|---|---|---|
| Bubble (visual) | visual vocabulary | analogy | The on-screen circle/region representing a scope or hypothesis a player/user can pop, distinct from `compiler/`'s executable `Bubble` struct (which is a real runtime scope object, not just a visual one — see the implementation-mechanism table entries for `compiler/`). | `demo.html` (`class SpherePop`), `game-engine.html`, `spellpop.html`, `memory-game.html` |
| Nesting (visual) | visual vocabulary | analogy | Visual containment of one bubble within another, used to suggest (but not implement) scope. | `demo.html`, `sandbox.html` |
| Popping animation | visual vocabulary | analogy | The UI transition played when a bubble is dismissed/committed; a rendering effect, not the Pop event. | `demo.html`, `game-engine.html`, `memory-game.html` |
| Field (visual) | visual vocabulary | analogy | A continuous background of drifting bubbles/hypotheses in `field-dynamics-simulator.html` and *Spherepop Trajectory Collapse*, illustrating an "informational gradient" — a metaphorical field, not the option space `Ω`. | `field-dynamics-simulator.html`; `Spherepop Trajectory Collapse.tex:117` |
| Trajectory | visual vocabulary | analogy | The path a player/vehicle/drone takes through a field of ambiguous hypotheses in *Spherepop Trajectory Collapse*; "bubble popping is framed as an explicit collapse operation" there, but the game itself does not implement the kernel's Collapse rule mechanism. | `Spherepop Trajectory Collapse.tex:77,89` |

## 7. Claims and analogies

These are illustrative or motivating statements about Spherepop's
relationship to ideas outside the calculus. They are not definitions to
be implemented, and should not be cited as if they were executable
facts — see the tracking issue's "cumulative evidence" framing.

| Claim | Status | Source |
|---|---|---|
| Types as refusal structures | analogy | The embedded SPC term calculus in the specification wraps a type in `Refused` under `Refuse`, suggesting refusal can carry typed information — but this is the spec's own inline sketch, explicitly scoped to term-level typing with "no event log, no replay, no notion of an arbiter." | `Spherepop_Specifications.tex:503,511` |
| Programs as histories | analogy | The general framing motivating the flat fixture suite: a program's meaning is its replayable event history, not merely its final state. Demonstrated concretely (not just claimed) by `experiments/flat/fixtures/05a`/`05b` (same terminal option space, different provenance). | `experiments/flat/README.md`; `experiments/flat/fixtures/05a_same_snapshot_different_history_run_a.json` |
| Proofs as replayable histories | analogy | Motivating language for the Phase F formal-assurance work (mechanizing invariants and connecting them to executable fixtures); no proof artifact exists yet — see `IMPLEMENTATIONS.md`'s "Formal specifications / Lean" entry (status: not started). | Tracking issue #1, Phase F checklist |
| Functions as history transformers | analogy | Used informally in the tracking issue's Phase C description of the "small abstraction" experiment; demonstrated concretely by `sugar::choice` being defined once and applied twice with different arguments in `experiments/flat/fixtures/12_small_abstraction.json`, not by any dedicated "function" construct in the kernel. | Tracking issue #1 §4; `experiments/flat/fixtures/12_small_abstraction.json` |

## How to extend this glossary

When adding a new document or implementation:

1. Look up every recurring term it uses against the tables above.
2. If the term already appears with the same meaning, cite this file
   instead of restating the definition.
3. If the term appears with a **different** meaning, add a row (or
   extend an existing row's "local synonyms / deviations" column) —
   do not silently assume the new usage matches the canonical one.
4. If the term is genuinely new, add it under the correct layer and
   assign it a status from the list at the top of this file.

Do not delete rows for superseded or historical usages; per the
tracking issue, discarded and divergent formulations are evidence of
the language's actual development, not noise to be cleaned up.
