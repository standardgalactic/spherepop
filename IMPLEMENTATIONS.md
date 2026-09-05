# Spherepop implementation manifest

This is the Phase A / Section 5 deliverable from issue #1 ("Establish
Spherepop as a language..."): an evidence-backed inventory of every
executable artifact in this repository that calls itself (or clearly
functions as) an implementation of Spherepop, classified as one of:

- **normative reference** — the executable counterpart of the current
  canonical specification (`SPEC.md`, the Phase B / Section 2 deliverable
  that freezes the four-primitive semantic nucleus); deviations from it
  are bugs.
- **conforming implementation** — implements the same four-primitive
  semantics as the normative reference and can be checked against shared
  fixtures.
- **experimental extension** — implements some version of the primitives
  (often a strict subset, or with real semantic gaps against the
  canonical spec) and is still under active development toward
  conformance.
- **historical artifact** — a real, executable prior formulation that
  predates the four-primitive consolidation and uses different (often
  incompatible) vocabulary; kept as evidence of the language's actual
  development, not silently reinterpreted to match the present spec.
- **visualization** — consumes or dramatizes Spherepop concepts for a
  human audience without implementing the primitive semantics itself.
- **domain application** — a runnable system built *on* some Spherepop-
  flavored model, exercised for its own sake rather than as evidence
  about the canonical calculus.

Every classification below is backed by reading the actual source
referenced, not by filename or documentation claims alone. Where a
file's own docs claim more than the code delivers, that gap is noted
explicitly, per the tracking issue's instruction not to silently rewrite
divergent or unfinished work to match the current specification.

## Canonical basis (for reference)

Per `Spherepop_Specifications.tex` and executed literally in
`spherepop-kernel`:

| Primitive | Canonical meaning |
|---|---|
| `Pop` | Commit to a specific option, removing it from what remains possible. |
| `Refuse` | Record an option as inadmissible without removing it from what remains possible. |
| `Bind` | Couple two elements as dependent without identifying them. |
| `Collapse` | Observe a history under a chosen rule by projecting it onto that rule's quotient space. |

`Sphere`, `Merge`, `Choice`, `Link`, `Unlink`, `Nest`, `SetMeta` are
derived surface sugar over these four in the canonical spec — not
separate primitives. Several implementations below predate that
consolidation and use `Merge`, `Pop`, or `Collapse` with unrelated,
older meanings; those are marked explicitly rather than described as if
they matched the current definitions.

## Inventory

### `spherepop-kernel/` (Rust) — **normative reference**

- Event alphabet is exactly `{Pop, Refuse, Bind, Collapse}`
  (`src/event.rs`), matching the canonical table above field-for-field:
  Pop shrinks `Ω`, Refuse leaves `Ω` untouched and only records a reason,
  Bind couples without identifying, Collapse is a pure external
  observation function over `History` (`src/collapse.rs`) that never
  mutates state.
- `Sphere`/`Merge`/`Choice`/`Link`/`Unlink`/`SetMeta` are implemented in
  `src/sugar.rs` as functions returning only primitive events — e.g.
  `Merge_c(a,b) := Collapse_c(Bind(a,b))` literally, not as a fifth event
  kind.
- The `Arbiter` (`src/arbiter.rs`) is the sole path by which history is
  extended; it enforces admissibility (Pop only within `Ω`, Collapse
  only under a certified rule, Refuse only with a nonempty reason).
- 20 unit tests cover primitive-level behavior, sugar/desugaring
  correctness, and kernel-level invariants (deterministic replay,
  irreversibility, conservation of the possibility functional, overlay
  preview/commit, observation non-interference).
- Additionally passes all 12 executable fixtures in `experiments/flat/`
  (see that directory), cross-checked against an independent Python
  oracle.
- **Why normative reference, not merely "a" conforming implementation:**
  it is dependency-free, deliberately kept line-by-line auditable against
  the LaTeX specification (see its own README), and is the implementation
  the flat fixture format and the Python oracle were designed against.

### `experiments/flat/run_python.py` — **conforming implementation**

- Independent, from-scratch reimplementation of the same four-primitive
  semantics (not a port — no code shared with the Rust crate), written
  specifically to cross-check `spherepop-kernel`.
- Agrees with the Rust kernel on all 12 executable flat fixtures.
- Scope is intentionally narrow: it only implements what the fixture
  suite exercises, not a general-purpose interpreter, REPL, or parser.

### `compiler/` (C) — **experimental extension**

- Real, large (lexer, parser, event-driven evaluator, history,
  constraints, provenance, admissibility gate, VM groundwork) with 10
  test files (`tests/lexer`, `tests/runtime`, `tests/admissibility`,
  `tests/integration/*.sp`).
- Internally uses an event bus with names close to, but broader than,
  the canonical four: `EV_POP_BEGIN`, `EV_POP_COMMIT`, `EV_REFUSE`,
  `EV_BIND`, `EV_COLLAPSE_APPLY`, plus lifecycle events (`EV_SCOPE_EXIT`,
  `EV_ADMISSIBILITY_RECALC`, `EV_CONSTRAINT_UPDATE`) that have no
  counterpart in the canonical spec (`src/runtime/evaluator.h`,
  `src/runtime/history.h`).
- **Known semantic deviations from the canonical spec** (not yet fixed;
  recorded here rather than silently assumed away):
  - `Bind` here performs constraint-set merging on a bubble
    (`cset_merge` + state transition), which is closer to "attach
    constraints to this scope" than "couple two elements as dependent
    without identifying them" (`src/runtime/bubble.c`).
  - `Collapse` is only partially wired: `bubble_collapse()` just marks
    a state flag and logs the event; the quotient-space semantics
    described in `src/runtime/collapse.h` are not yet connected to the
    evaluator (`src/runtime/evaluator.c`).
  - There is no explicit `Ω`/option-space type; the closest analogue is
    an "admissibility manifold" over bubble state (`bubble.h`,
    `admissibility.h`).
- `ROADMAP.md` self-reports Phase 1 ("Operational Core") complete and
  Phase 2+ (region allocator, bytecode compiler, manifold
  representation, visualizer) unchecked — this project explicitly
  considers itself pre-conformance, not a finished reference.
- **Now builds and passes its own test suite** (`cmake .. && make &&
  ctest`, 5/5 green — two build-blocking bugs fixed this session, see
  "Follow-up work" below), but the interpreter itself is still **not**
  wired to `experiments/flat/`: doing so meaningfully would require
  completing `Bind`/`Collapse` against the canonical semantics first
  (fixture failures before that point would just restate the known
  gaps above), which in turn needs an explicit Ω/option-space concept
  this interpreter doesn't have yet.
- A **separate, standalone C port** of the canonical model now exists
  at `compiler/tools/fixtures/` (built as the `sp_fixtures` CMake
  target) specifically to give C a genuine fixture adapter without
  waiting on that redesign — see "Follow-up work" item 2 and
  `experiments/flat/CONFORMANCE.md`. It shares no code with, and does
  not change the status of, the Bubble-based interpreter described
  above.

### `prototypes/sphereforth_gforth.zip` (Forth, gforth) — **experimental extension**

- A small (~150 lines across `ops.fs`, `log.fs`, `sym.fs`, `main.fs`,
  `repl.fs`, `demo.fs`), self-described "thermodynamics-first" prototype.
- Vocabulary matches the canonical primitives unusually closely for a
  historical/independent artifact: `OPT` (introduce an option), `POP`
  (irreversibly drop/commit an option), `REFUSE` (same stack effect as
  POP, "different accounting label" — i.e. explicitly does not conflate
  the two), `BIND` (log a dependency, keeps both options on the stack),
  `COLLAPSE` (replace two options with a synthesized quotient token,
  logs the quotient) — see its own `README.md` inside the archive.
- Implements the append-only log and replay directly: the log is stored
  as literal Forth source text and `REPLAY` clears the stack and
  re-executes it, which is a concrete (if literal-minded) demonstration
  of deterministic replay from a log.
- Self-described limitations (from its own README): "Not SPC. Not a
  full type system. Not a 'nice' user language" — i.e. it explicitly
  disclaims being a complete conforming implementation.
- Could not be executed in this environment (the sandboxed session has
  no working `gforth` runtime path — `gforth` is installed as a snap but
  fails with "cannot join mount namespace of pid 1" here); the source
  was read and manually traced instead. Running it and wiring a fixture
  adapter for `experiments/flat/` is good follow-up work given how close
  its vocabulary already is to the canonical one.

### `reduction-engine/spherepop.py` — **domain application / experimental extension**

- 739-line, runnable, tested-by-demo (`python3 spherepop.py --demo`)
  reduction engine built around "Bubbles": concurrent, message-passing
  processes with their own instruction set (`set`, `add`, `mul`, `div`,
  `sqrt`, `send`/`recv`, `spawn`, `collapse`, `halt`).
- Uses `POP`/`REFUSE`/`COLLAPSE` as *outcome labels* for each reduction
  step, not as the canonical primitives:
  - `POP` here means "this instruction reduced successfully," not
    "commit to a specific option, removing it from what remains
    possible."
  - `REFUSE` means "the admissibility gate rejected this reduction,"
    which is compatible in spirit but is a per-instruction control-flow
    outcome, not a standalone event over an option space.
  - `COLLAPSE` means "commit a value upward to the parent bubble," a
    return-value mechanism, not a quotient-space observation over
    history.
  - There is no `Bind` primitive at all, and no explicit `Ω`.
- Confirmed executable: running `--demo` in this session reproduced all
  four documented demo scenarios (POP chain, REFUSE on bad sqrt, REFUSE
  on divide-by-zero, COLLAPSE escaping a value upward) exactly as its
  own docstring describes.
- Genuinely useful and self-contained, but its vocabulary overlap with
  the canonical primitives is a false-friend risk worth flagging in the
  glossary work (Section 6 of the tracking issue): a reader who knows
  the canonical semantics would misread this engine's `POP`/`COLLAPSE`.

### `prototypes/python/spherepop/` (`core.py`, `parser.py`, `repl.py`) — **historical artifact**

- Small (~200 lines total), built around a `Region`-based model:
  `Region(label, payload)`, `Atom`, `Pop`, `Merge` term constructors, and
  a `collapse` function that is just a strategy parameter (default:
  flatten payload one level) — see `core.py`.
- `Pop` here means "evaluate this subterm, then run the collapse
  strategy over it" (ordinary reduction), not "commit to an option,
  removing it from Ω." There is no `Refuse`, `Bind`, or event-log/history
  concept anywhere in the module.
- Has its own tests (`tests/test_core.py`, `tests/test_parser.py`) that
  pass against its own (older) semantics — this is a coherent, tested
  artifact, just one that predates and diverges from the four-primitive
  consolidation. Recorded here rather than silently reinterpreted.

### `spherepop.hs` (Haskell) and `spherepop.rkt` / `main.rkt` (Racket) — **historical artifacts**

- Two independent-language renderings of the *same* older calculus:
  term constructors `Var`, `Sphere`, `Pop`, `Merge`, `Choice`, `Rotate`,
  plus literals/`If`/`Add` (`spherepop.hs:30-42`, `spherepop.rkt:19-29`).
- `Pop` = ordinary function application; `Merge` = tensor/pairing;
  `Choice` = probabilistic mixing over a type/distribution — none of
  which match the canonical Pop/Refuse/Bind/Collapse vocabulary or an
  event-log/history model. There is no `Refuse`, `Bind`, `Collapse`, or
  `Ω` in either file.
- Both are executable with worked demo terms (`demoMergeCoins`,
  `demoDoomTensor`, etc. in Haskell; `demo-merge-coins`,
  `demo-doom-tensor`, etc. in Racket) and appear to be a matched pair —
  the same calculus implemented twice for cross-language comparison,
  predating the current canonical semantics rather than conforming (or
  failing to conform) to it.

### HTML/JS demos (`demo.html`, `game-engine.html`, `spellpop.html`, `memory-game.html`, `field-dynamics-simulator.html`, `sandbox.html`, `index.html`, and related) — **visualization**

- Large (6,000+ combined lines), interactive, browser-based. `demo.html`
  defines a `class SpherePop { ... }` (`demo.html:391`), but grepping for
  primitive-shaped method calls (`pop(`, `bind(`, `collapse(`,
  `refuse(`, `merge(`) across these files turns up no calculus API —
  the one `.pop()` hit in `demo.html` is a plain JS array truncation for
  a UI history buffer, not an event primitive.
- These are genuinely useful as domain/visual demonstrations of the
  *bubble/sphere metaphor* (nesting, popping animation, field dynamics)
  but do not implement or claim to implement the primitive semantics;
  they should be paired with a conforming core/adapter if they are ever
  used as language-conformance evidence (per Section 5 of the tracking
  issue), not treated as evidence on their own.

### Formal specifications / Lean — **unresolved / not yet started**

- No `.lean` files exist anywhere in the repository (confirmed via
  repository-wide search).
- Several `.tex`/`.md` documents (e.g. under `architecture-of-meaning/`,
  `framework/`, `textbook/`) *mention* Lean or "mechanized proofs" as a
  goal or reference point, but no proof scaffolding, statement, or build
  exists yet.
- This is Phase F work in the tracking issue ("State and prove ...",
  "Mechanize the stable core") and should be tracked as not started,
  rather than implied to exist because it is discussed in prose.

## Summary table

| Implementation | Location | Status |
|---|---|---|
| Rust kernel | `spherepop-kernel/` | Normative reference |
| Python oracle | `experiments/flat/run_python.py` | Conforming implementation |
| C fixture kernel | `compiler/tools/fixtures/` | Conforming implementation |
| C compiler/runtime | `compiler/` (interpreter proper) | Experimental extension (builds + own tests pass; Bind/Collapse incomplete, not wired to fixtures) |
| SphereForth | `prototypes/sphereforth_gforth.zip` | Experimental extension (unexecuted here; vocabulary closely aligned) |
| Bubble reduction engine | `reduction-engine/spherepop.py` | Domain application / experimental extension (false-friend vocabulary) |
| Python `Region` prototype | `prototypes/python/spherepop/` | Historical artifact |
| Haskell calculus | `spherepop.hs` | Historical artifact |
| Racket calculus | `spherepop.rkt`, `main.rkt` | Historical artifact |
| HTML/JS demos | `demo.html`, `game-engine.html`, etc. | Visualization |
| Lean/formal proofs | — | Unresolved / not started |

## Follow-up work this inventory implies

1. **`compiler/`'s build is now fixed (done this session).** Two
   independent, genuine bugs blocked it, both confirmed by direct
   attempt and fixed in this tree:
   - `CMakeLists.txt` listed `src/parser/precedence.c` and
     `src/parser/prettyprint.c` as sources, but neither file exists.
     Their would-be functionality already lives elsewhere (precedence
     climbing is implemented inline in `parser.c`'s `parse_unary` /
     `parse_power` / `parse_mul` / ... chain; AST pretty-printing is
     `node_print()` in `ast.c`), so the two nonexistent entries were
     simply dead references — removed from `CMakeLists.txt`.
   - `tests/CMakeLists.txt` did not exist at all, even though the
     default `SP_ENABLE_TESTS=ON` unconditionally calls
     `add_subdirectory(tests)` — meaning the project could not even be
     *configured* with its own default options. Added a
     `tests/CMakeLists.txt` that builds the five existing
     `test_*.c` files (already self-contained, already returning 0/1)
     as CTest cases.
   - One further pre-existing bug surfaced once tests could run: the
     parser requires parenthesized `if (cond) { ... }`, but
     `tests/integration/test_programs.c` itself used paren-less
     `if 1 { 99 } else { 0 }` and could never have passed. Fixed the
     test fixtures to use the grammar the parser actually implements.
   - Verified: `cmake .. && make && ctest` now succeeds end-to-end,
     5/5 test suites pass, and `./spherepop -e "1 + 2"` /
     `./spherepop --ast -e "pop(1+2)"` both run correctly.
   - See `experiments/flat/CONFORMANCE.md`'s "Blocked adapters" section
     for the current, narrower status.
2. **Done — a fixture adapter for C now exists, as a standalone port,
   not as a `compiler/` language feature.** `compiler/`'s own
   Bubble/EvalContext runtime still has the semantic gaps described
   above (`Bind` = constraint-set merge, `Collapse` = unimplemented —
   `NODE_BIND`/`NODE_COLLAPSE` are not even handled in `eval_node`'s
   switch, so no `.sp` script can exercise them at all yet), and fixing
   *that* would mean redesigning the interpreter's object model around
   an explicit option-space/Ω concept it does not currently have. Rather
   than block a fixture adapter on that redesign, `compiler/tools/
   fixtures/` is a from-scratch, ~800-line C port of the same canonical
   event/history/arbiter/collapse/sugar model already ported to Rust
   (`spherepop-kernel/src/{event,history,arbiter,collapse,sugar}.rs`)
   and Python (`experiments/flat/run_python.py`) — its own dependency-
   free JSON parser, a linear-array `ObjectSet`/`BoundList`/
   `RefusedList`/`ObservedList`, an `Arbiter` with atomic
   validate-then-append `submit()`, union-find-based quotient collapse
   rules, and the five sugar forms (`link`/`unlink`/`choice`/`merge`/
   `set_meta`), built as the `sp_fixtures` CMake target and deliberately
   **not** linked against `spherepop_core`. All 12 executable fixtures
   pass and agree with Rust and Python; verified additionally clean
   under `-fsanitize=address,undefined`. See
   `experiments/flat/CONFORMANCE.md` for the live 3-way matrix.
   Closing the *interpreter's own* Bind/Collapse gap so that `.sp`
   source programs (not just the abstract fixture format) can express
   the canonical primitives remains open and is a separate, larger
   redesign task.
3. Get a working Forth runtime in a suitable environment and wire
   `prototypes/sphereforth_gforth.zip` to the flat fixtures — its
   vocabulary is already the closest historical artifact to the
   canonical primitives. (Attempted here: blocked by a snap-confinement
   failure and a missing `libtool` needed by gforth 0.7.9's own FFI
   bootstrapping at startup — see `experiments/flat/CONFORMANCE.md`.)
4. Fold the vocabulary false-friends found here (`reduction-engine`'s
   outcome-label `POP`/`COLLAPSE`; the Haskell/Racket calculus's
   unrelated `Pop`/`Merge`/`Choice`/`Rotate`) into the glossary work
   from Section 6 of the tracking issue, explicitly marking them as
   local synonyms with different meanings rather than leaving the
   overlap unexplained. **(Done — see `GLOSSARY.md`.)**
5. Start the Lean/formal-assurance track from zero rather than assuming
   any existing scaffolding — Phase F has no code basis yet.
