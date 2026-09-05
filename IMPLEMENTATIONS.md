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
- Additionally passes all 15 executable fixtures (13 numbered, 05 and 13 each a pair) in `experiments/flat/`
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
- Agrees with the Rust kernel on all 15 executable flat fixtures (`05` and `13` are each a two-file pair), including the new `13a`/`13b` pair — the Rust kernel was actually linked and run here via a static-musl/`rust-lld` workaround for the missing system `cc` (see `experiments/flat/CONFORMANCE.md`), not merely type-checked.
- Scope is intentionally narrow: it only implements what the fixture
  suite exercises, not a general-purpose interpreter, REPL, or parser.

### `spherepop-go/` (Go) — **conforming implementation**

- Independent, from-scratch Go 1.22 reimplementation of the canonical
  four-primitive semantics (`Pop`, `Refuse`, `Bind`, `Collapse`); shares
  no code with the Rust crate, the Python oracle, or `compiler/`
  (`spherepop-go/event.go`, `spherepop-go/history.go`).
- Closed event alphabet with pointer fields to distinguish "absent" from
  "zero value" (`spherepop-go/event.go:36-43`); append-only `History`
  with defensive-copy accessors (`spherepop-go/history.go:83-93`).
- `Arbiter` performs atomic proposal admission: a `Proposal`'s events
  are validated as a batch against the state produced by replaying each
  preceding event in the same proposal, then appended all-or-nothing
  (`spherepop-go/arbiter.go:57-95`); certified `Collapse` rules are
  checked against an explicit allow-list, matching the Rust kernel's
  rule-certification requirement.
- Structural `Meld` composes two histories by concatenation and
  position renumbering, not a fifth primitive
  (`spherepop-go/history.go:104-111`); `CollapseQuotient` /
  `CollapseQuotientHonoringRefusals` implement the union-find quotient
  and its refusal-aware variant (`spherepop-go/collapse.go`).
- Derived surface operations (`Link`, `Unlink`, `Choice`, `Merge`,
  `SetMeta`) are implemented purely as combinations of the four
  primitives, matching the same desugaring documented for
  `experiments/flat/run_python.py` — `Link`→`Bind`, `Unlink`→`Refuse` of
  a withdrawn relation, `Choice`→`Pop`+`Refuse`, `Merge`→`Bind`+
  `Collapse`, `SetMeta`→a `Bind` tagged `"__meta__"` and excluded from
  the ordinary quotient (`spherepop-go/sugar.go`).
- A non-authoritative `OverlayManager` previews a pending proposal's
  replayed state without mutating the arbiter's history, and rejects
  `Commit` if the underlying history advanced since the overlay was
  created (`spherepop-go/overlay.go`) — this is a genuinely distinct
  capability not present in the Python oracle or the C fixture adapter.
- Own unit test suite (`spherepop-go/kernel_test.go`, `go test ./...`)
  covers primitive replay, atomic-proposal rejection, meld+quotient,
  and overlay staleness — verified green in this session
  (`ok github.com/standardgalactic/spherepop/spherepop-go`).
- Ships its own fixture runner (`spherepop-go/cmd/fixtures/main.go`,
  `go run ./cmd/fixtures ../experiments/flat/fixtures`) consuming the
  same shared JSON fixtures as Rust/Python/C. Verified in this session:
  **15/15 fixtures pass**, including the `13a`/`13b` quotient-equality
  pair — a fourth independent implementation now agrees with the
  canonical suite. `go vet` and `go build ./...` are clean; `gofmt`
  formatting was applied (whitespace-only) during this verification.
- Scope, like the Python oracle, is intentionally narrow: it implements
  what the fixture suite exercises plus the overlay mechanism, not a
  parser or REPL.

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

### `prototypes/sphereforth_gforth.zip` (Forth, gforth) — **conforming implementation (fixed and executed this session)**

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
- **Now executed and fixed this session.** `gforth` (installed as a
  snap) fails normally here with "cannot join mount namespace of pid 1"
  under snap confinement, and even the underlying `gforth-fast` binary's
  default startup image tries to dynamically compile and libtool-link a
  small C shim for its FFI bindings — impossible with no `cc`/`libtool`
  present. Workaround: invoking `gforth-fast` with the *bare/minimal*
  bootstrap image (`--image-file=.../kernl64l.fi`) skips that FFI
  bootstrap entirely and provides a working (if minimal — missing
  `char`/`[char]`, itself standard ANS Core vocabulary just absent from
  this stripped image, polyfilled for test-harness use only) Forth
  environment. Reproduction:
  ```sh
  LD_LIBRARY_PATH=/snap/gforth/34/usr/lib/x86_64-linux-gnu:/snap/gforth/34/lib/x86_64-linux-gnu \
    /snap/gforth/34/usr/bin/gforth-fast \
    --image-file=/snap/gforth/34/usr/lib/gforth/0.7.9_20250321/kernl64l.fi \
    demo.fs
  ```
- Running the actual shipped `demo.fs` this way surfaced **five genuine,
  pre-existing bugs**, independent of this sandbox — each verified by
  isolating the offending word, observing the crash/wrong-output, then
  re-testing the fix in isolation before applying it:
  1. `log-append` (`log.fs`) consumed its `u` (length) argument in
     `move`, then tried to reuse it to advance `log-pos` — the stack
     only had the *old* `log-pos`, not `u`, left; `+` then underflowed.
     Fixed by saving `u` on the return stack across the `move`.
  2. `log-line` (`log.fs`) wrote a log-line terminator via
     `s" \n" log-append`; plain `S"` never processes backslash escapes
     (ANS-standard), so this appended the two literal characters `\`
     and `n`, not a newline byte. Fixed by writing byte value 10
     directly.
  3. `heap-store` (`sym.fs`) computed a destination address, then
     shuffled the stack for `move` in the wrong order — its `move` call
     ended up reading the string's *length* as `move`'s destination
     address, corrupting memory (verified: a `SIGSEGV`, error `#-9`).
     Fixed by stashing the destination address in a scratch variable
     rather than juggling it across `>r`/`r@`.
  4. `set-name` (`sym.fs`) called `heap-store` with `id` still sitting
     on top of the data stack instead of `c-addr u`, again corrupting
     memory (`SIGSEGV`). Fixed by stashing `id` in a scratch variable
     while `heap-store` runs, matching the pattern above.
  5. `OPT` and `COLLAPSE` (`ops.fs`) both used `r@ -rot set-name` to
     restore a saved id, which peeks the id without popping it and
     leaves a stray `c-addr` on the stack, feeding `set-name` the wrong
     three arguments; `COLLAPSE` additionally had a stray `2dup` after
     `next-qname` that left an unconsumed name string on the data
     stack it never used it. Fixed by using a plain `-rot`/`r>` and
     removing the unnecessary `2dup`.
  6. `next-qname` (`ops.fs`) built its numeral suffix with
     `<# #S #>`, which expects a genuine double-cell (`ud`) number;
     the code instead padded a single-cell counter with a stray pad
     address (from `1 pad +`) rather than `0` as the high cell,
     producing garbage digits (verified: `qN` came out as `q` followed
     by a raw memory address, not `q0`). Replaced with a small
     hand-written `num-to-qbuf` digit formatter that avoids `<# #S #>`
     entirely.
  7. `HELP` (`repl.fs`) embedded a literal `\" ` inside a plain
     `."..."` string to print `` S" name" OPT ``; plain `."` never
     processes backslash escapes either (the same root cause as bug 2
     above), so the string terminated at the first raw `"` and left
     `name\" OPT ...` as unparsed source, crashing with "undefined
     word" — this means `HELP`, and therefore `run`'s REPL loop (which
     calls `HELP` first), would crash in *any* gforth, not just this
     sandbox. Fixed by emitting the quote character separately via a
     small `qchar` helper.
- With all seven bugs fixed, the shipped `demo.fs` was run to
  completion end to end (fresh extraction of the updated archive,
  verified byte-identical output across repeated runs): two options
  are opened and bound, one is popped, two more are opened and
  collapsed into `q0`, and `.STACK`/`.LOG` show exactly the expected
  final frontier (`q0`, `door-sill:pine`) and the expected six-line
  executable log. `REPLAY` was also verified: it reproduces the
  identical final stack from the log alone.
- **What remains unverified**: the interactive `run` word in `repl.fs`
  (invoked by `main.fs`) could not be exercised end-to-end here — this
  bare/minimal bootstrap image's `refill` does not read piped stdin the
  way a full gforth image's terminal-input machinery would (confirmed:
  `refill` returns false immediately even with buffered input waiting),
  which appears to be a further consequence of using the stripped-down
  image to work around the missing C toolchain, not a SphereForth bug.
  `HELP` and `RESET` (the words `run` calls) were verified directly by
  calling them standalone, which is how bug 7 above was found and
  confirmed fixed.
- The fixed archive (`log.fs`, `sym.fs`, `ops.fs`, `repl.fs` changed;
  `README.md`, `main.fs`, `demo.fs` unchanged) replaces the original
  `prototypes/sphereforth_gforth.zip` in this repository. Wiring a
  fixture adapter for `experiments/flat/` remains good follow-up work
  given how close its vocabulary already is to the canonical one — see
  "Follow-up work" below.


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
| Go implementation | `spherepop-go/` | Conforming implementation |
| C fixture kernel | `compiler/tools/fixtures/` | Conforming implementation |
| C compiler/runtime | `compiler/` (interpreter proper) | Experimental extension (builds + own tests pass; Bind/Collapse incomplete, not wired to fixtures) |
| SphereForth | `prototypes/sphereforth_gforth.zip` | Conforming implementation (7 pre-existing bugs found and fixed this session; `demo.fs`/`REPLAY` verified end-to-end via a bare-kernel gforth workaround; interactive REPL loop unverified) |
| Bubble reduction engine | `reduction-engine/spherepop.py` | Domain application / experimental extension (false-friend vocabulary) |
| Python `Region` prototype | `prototypes/python/spherepop/` | Historical artifact |
| Haskell calculus | `spherepop.hs` | Historical artifact |
| Racket calculus | `spherepop.rkt`, `main.rkt` | Historical artifact |
| HTML/JS demos | `demo.html`, `game-engine.html`, etc. | Visualization |
| Lean/formal proofs | — | Unresolved / not started |

## Tenth lineage, external to this repository

`GRAMMAR.md` (added later, consolidating a candidate grammar for a
sphere-expression/nested-quotient calculus) verifies its claims against
`test_spherepop.py` and `spherepop-os.tex` — files pasted into this
repository's root by the user, but whose actual implementation package
(`grammar.py`, `model.py`, `semantics.py`, `parser.py`, `views.py`,
`validation.py`, `poset.py`, plus 29 numbered experiment subdirectories)
lives only in a separate repository, `standardgalactic/personalinks`,
at `personalinks/spherepop/`. Confirmed via the GitHub API:
`personalinks/tests/test_spherepop.py` is byte-identical (modulo
formatting) to the copy pasted here, and `personalinks` has its own
`spherepop-os.tex`/`.pdf`, Commodore tutorial files, and
`analyze-spherepop.sh` at its repository root. This is a real,
substantial tenth Spherepop lineage, but it is **not** an executable
artifact of this repository and is intentionally not added to the
inventory or summary table above (which is scoped to artifacts
`present and running in this repository`). It uses the same four
primitive names (`Pop`/`Refuse`/`Bind`/`Collapse`) as the canonical
basis above but with a genuinely different semantics (nested
sphere-path/quotient grammar vs. this repo's integer-object-id
option-space/history model) — see `GRAMMAR.md`'s provenance note for
the fuller account of this unreconciled conflict.

## Eleventh lineage, external to this repository

`calculus/identity_after_collapse.tex`, in `standardgalactic/calculus`
(byte-identical to `calculus/working/Identity_After_Collapse.tex` in the
same repository — same Git blob SHA
`7389225cedb3227319f7b390d67e71f4896cbf70`, confirmed via the GitHub
API), is a **genuine, source-verified external formal specification /
non-canonical bubble lineage**, not an implementation of this
repository's canonical calculus and not an executable artifact of any
kind — no code listing, `lstlisting`, or `verbatim` block of any
language appears anywhere in its 4051 lines; it is LaTeX prose,
definitions, theorems, and TikZ figures only.

**Primitive macros.** The document defines `\pop`, `\refuse`, `\bind`,
and `\collapseop` (all `\mathsf{...}` typographic macros, `\S1.2`), so
it does use all four canonical primitive *names*. However, unlike
`SPEC.md`, these four names are **not given one unified typing/
operational-semantics definition set applying uniformly to a single
state model.** Only `\collapseop` and the underlying evaluation map
$\evalf$ are formally defined in the calculus's own foundational
chapters (Ch. 2–4); `\pop`, `\refuse`, and `\bind` are introduced
informally and reappear with *different* glosses in later, more
applied chapters:
- In the "Historical Lexical-Functional Grammar" application (Ch. 6),
  `\bind_R` narrows a "grammatical option space" $X$ by a relation $R$,
  and `\refuse` eliminates trajectories on a feature mismatch (an
  operational role that does resemble `SPEC.md`'s `Bind`/`Refuse`).
- In the RSVP physical-field appendix, `\bind` "constrains compatible
  regions" of a scalar field, `\pop` "eliminates unstable
  configurations, collapsing regions of high curvature," and `\refuse`
  "removes incompatible trajectories, increasing the entropy gradient"
  — an entirely different (field-dynamical) gloss for the same three
  names.

There is no single chapter in which `Pop`, `Refuse`, `Bind`, and
`Collapse` are presented together as four co-equal, uniformly-typed
primitives over one explicit state — this is a structural difference
from `SPEC.md`'s presentation, not merely a difference in what the
primitives compute.

**State model.** The base calculus (Ch. 2–4) is a term-rewriting
system, not an option-space/event-log kernel. Its objects are
*Spherepop expressions*: atomic tokens, or parenthesized groupings
$(E_1 \cdots E_n)$ called **bubbles** — "a bounded region of structural
commitment," consumed once reduced and not retained in the result.
Reduction is governed by a rule set $\mathcal{R}$ (assumed confluent
and terminating), and evaluation is the induced canonical map
$\evalf : \Expr \to \NF$, giving the quotient $\NF \cong \Expr/{\sim}$.
The **history** of a computation is the *event word* $\partial(E) \in
\Sigma^*$ — the recorded sequence of reduction steps — which is
explicitly distinguished from, and richer than, the final *trace*
$\evalf(E)$ that identity is defined over.

**Difference from `SPEC.md` (canonical basis).** Two differences are
worth flagging as more than cosmetic:
1. *No single unified state.* `SPEC.md`'s four primitives act on one
   object with an admissible option space and an append-only history
   (formalized in `spherepop-os.tex`'s "State Term, Extended" as
   $t = (I_t, Opt_t, H_t)$, already cited in `COMPLEXITY.md` §13). This
   document's "state" is instead a syntactic expression under generic
   term rewriting, with `Pop`/`Refuse`/`Bind` functioning as
   chapter-local glosses over that rewriting system rather than as
   primitives of one formal kernel.
2. **Inverted identity philosophy.** This is the sharpest divergence.
   `SPEC.md`/`GRAMMAR.md`/`COMPLEXITY.md`'s canonical thesis is that
   identity is *historical*: two things are the same only if they
   share the same event history (the whole point of the 05a/05b
   fixture pair — same snapshot, different history, therefore *not*
   the same object). `identity_after_collapse.tex` states the opposite
   as its own explicit thesis (Ch. 5, "Trace and Identity," Definition
   "Trace Identity"): two expressions are **co-identical if they share
   the same trace**, explicitly *regardless* of whether their event
   words (histories) differ — "shared traces do not entail shared
   histories... Identity is defined solely at the level of the trace,"
   i.e. an *extensional*, not historical, identity criterion. This
   document is not merely a different primitive grammar; it is a
   different foundational commitment about what identity means, argued
   for on its own terms rather than left implicit.

**Relationship to the "History as Identity" family (family 6 above).**
Not a direct textual relative — no citation to `History as
Identity.tex`/`-v01`/`-v02` was found, and the two documents are not
byte-related. But they are worth contrasting precisely because family
6's `Pop`/`Bind`/`Collapse` (no independent `Refuse`) already operates
over an explicit option-space model closer in spirit to `SPEC.md` than
to this document's bubble-rewriting system — `identity_after_collapse
.tex` is a third, independent treatment of "identity under
irreversible evaluation," arriving at the *opposite* conclusion about
what survives (trace/extension) from what family 6 and the canonical
basis both assume (history/intension). All three lineages use
overlapping vocabulary for genuinely different formal claims; none
should be read as a variant or refinement of either of the other two.

**Label:** external formal specification / non-canonical bubble
lineage. Not an implementation of anything in this repository, and not
reconciled with `SPEC.md`, `spherepop-os.tex`, or family 6 — recorded
here as a documented, source-verified external lineage only, per
`EXTERNAL-SPHEREPOP-REFERENCES.md` item 1.

## Specification lineages (the `.tex` corpus)

This inventory's nine executable artifacts are matched against far more
*specification* documents than the two ("canonical" vs. "`personalinks`")
lineages discussed above. A full-content pass over every one of the
repository's 115 tracked `.tex` files (112 distinct after collapsing
byte-identical duplicates by sha256 — see `scripts/analyze_tex_corpus.py`,
which automates the dedup and a first-pass grammar/formal-content signal
scan so this triage doesn't rely on filenames or memory) found
**approximately seven distinguishable specification families**, not
merely two:

1. **The canonical four-event option-space/history calculus** —
   `Spherepop_Specifications.tex` (duplicated byte-for-byte at
   `processing/Spherepop_Specifications.tex`). Explicitly states it
   "supersedes the primitive vocabulary of earlier Spherepop documents";
   fixes `Pop`/`Refuse`/`Bind`/`Collapse` as the only primitives with
   `Sphere`/`Merge`/`Choice`/`Link`/`Unlink`/`Nest`/`SetMeta` as sugar;
   gives an EBNF geometric DSL, typing rules, desugaring rules,
   operational semantics, an event algebra, ABI layouts, and reference
   Haskell/Rust material. This is the document `spherepop-kernel/`,
   `experiments/flat/run_python.py`, and `compiler/tools/fixtures/`
   actually implement (see "Canonical basis" above).
2. **The OS event/state-transition layer** — `spherepop-os.tex` (kernel
   state $\sigma=(O,U,R,M)$, an authoritative event log, transition
   relations, deterministic replay, snapshots/non-authoritative views,
   and reductions of OS-level events into the causal calculus above).
   `Spherepop-OS.tex` (uppercase) is an older, much shorter ancestor with
   the basic state/transition model but not the later reductions or
   extended term grammar — treat these as one OS lineage with the
   lowercase file authoritative (see `GRAMMAR.md`).
3. **The nested-sphere/path/quotient grammar** imported from
   `standardgalactic/personalinks` (`test_spherepop.py`, verified against
   `spherepop-os.tex` in `GRAMMAR.md`) — see "Tenth lineage" above; not
   implemented anywhere in this repository.
4. **The historical Sphere–Pop–Merge–Choice calculus** —
   `monograph/spherepop-calculus.tex`, `textbook/beyond-parentheses.tex`,
   `essay/spherepop_calculus.tex`, `essay/draft-01/spherepop_calculus.tex`,
   `essay/draft-01/essay.tex`, `spherepop-foundations.tex`. Grammars,
   typing/reduction rules, probabilistic `Choice`, application-like
   `Pop`, sometimes with Haskell or Racket listings — the conceptual
   counterpart of the historical Haskell/Racket/Python-`Region`
   implementations above, not the current four-event alphabet.
5. **The geometric Merge–Collapse calculus** — `processing/essay.tex`
   ("Spherepop: A Language for Geometric Computation — Unified Rigorous
   Specification": lexical syntax, EBNF, operator precedence, typing,
   translation semantics, operational and denotational semantics, an
   implementation roadmap) and `computing-with-spherepop.tex` (a similar
   merge/collapse language with finite-state, Boolean-circuit, and
   lambda-calculus encodings). Substantial enough not to dismiss as
   ordinary essays, but a different primitive ontology than family 1.
6. **The three-event History-as-Identity algebra** — `History as
   Identity.tex` (`Pop`/`Bind`/`Collapse` only, no autonomous `Refuse`;
   `Bind` restricts the option space, `Pop` records commitment,
   `Collapse` reconstructs observable state; abstract syntax and typing
   judgments; proves this 3-operator set minimal/generating for its own
   history algebra), with `History as Identity - v01.tex` / `- v02.tex`
   documenting its evolution. An important precursor to family 1's
   option-space model, but not the same operator set.
7. **The five-constructor historical type calculus** —
   `textbook/dependent-type-theory.tex` (`Pop`, `Refuse`, `Bind`,
   `Collapse`, **and `Meld`** as a fifth constructor, each with typing
   rules, plus historical contexts, dependent products, universes,
   equality types, inductive families, normalization, and a
   bidirectional type-checker design) — **the most extensive typed
   specification of a Spherepop language in the repository, and
   currently unimplemented anywhere in this repository.** It directly
   conflicts with family 1's rule that `Meld` is history composition
   (sugar), not a fifth primitive (`SPEC.md`; see the Meld fixture in
   `experiments/flat/`). This should be recorded as a **specified but
   unimplemented typed extension** — a real prospective implementation
   target, not "just an essay." A more distant relative,
   `dynamics/spherepop_scope_dynamics.tex`, defines its own five-operation
   algebra (`Open`, `Pop`, `Meld`, conservative `Reframe`, expansive
   `Reframe`) as a containment-topology/cognitive-dynamics calculus that
   explicitly claims primitive status and completeness, but is not an
   implementation specification for the language above.

Additional documents with real formal content that support one of the
families above rather than defining an independent one:
`textbook/refusal-structures.tex` (a less-settled calculus-of-constructions
proposal directly relevant to any future type checker, closely related to
family 7); `identity_as_event_history.tex`, `Joy-of-Spherepop.tex`,
`Structured Irreversibility.tex`, `throwing_the_game_v2.tex`,
`Event-historical Aggregation.tex`, and `admissibility/spherepop-monograph.tex`
(formal syntax, event semantics, reduction rules, categorical structure,
option-space semantics, or invariants — some with locally incompatible
meanings, e.g. `throwing_the_game_v2.tex` treats `Collapse` as uniquely
able to expand admissibility). `history/The History of Spherepop.tex`
includes a BNF appendix ("Appendix G") that explicitly disclaims being
"a concrete syntax for any particular implementation," i.e. it documents
history rather than specifying an implementation target. The remaining
Python/Haskell/Racket-focused `.tex` tutorials describe the historical
prototypes already listed in the inventory above rather than defining
additional implementations.

**Net effect on this manifest:** the executable-implementation count
above remains **nine** (ten counting the external `personalinks`
lineage) — this section does not add new executable artifacts. What it
corrects is the impression that only two specification lineages exist;
there are at least seven, several of them (`textbook/
dependent-type-theory.tex` above all) substantial enough to be real
prospective implementation targets rather than terminology-only essays.
Reconciling or explicitly cataloguing all seven remains open work under
issue #1.

### Addendum: `scripts/tex-spec-query.sh` findings (genuine hits vs. false positives)

A second, independent scanning tool, `scripts/tex-spec-query.sh` (a
keyword/structure-scoring shell script, distinct from
`scripts/analyze_tex_corpus.py` above), was run over the same 115
tracked `.tex` files and ranked them by a score built from
`\begin{definition}`/`\begin{theorem}` counts, grammar/BNF keyword
hits, and raw counts of the words `Pop`/`Refuse`/`Bind`/`Collapse`/etc.
Its top-ranked results were checked by hand against actual file
content, which surfaced both real gaps in the seven-family list above
and a scoring weakness worth recording so it isn't repeated:

**Genuine additional Spherepop content the seven-family pass above did
not mention** (none of these were previously catalogued anywhere in
this manifest):
- `textbook/The_Ecology_of_Distinctions.tex` — contains an actual
  **Chapter 24, "Spherepop"** (`\chapter{Spherepop}`), which states it
  will "define the four Spherepop primitives" and connect them to
  operational semantics.
- `intelligence-explosion.tex` (byte-identical duplicate at
  `processing/intelligence-explosion.tex`) — contains a worked-example
  section, "A Worked Example: Spherepop over [an ambient probabilistic
  substrate language]," describing "Spherepop events" as irreversible
  constraints and a "Quantum SpherePop" connection.
- `Operational-Mereology.tex` — argues that "the Spherepop calculus and
  kernel provide a viable alternative foundation grounded in
  mereology," defining an operational containment relation
  $x \le y$ over replayed event history.
- `processing/geometry/monograph.tex` ("The Geometry of Spherepop") —
  extends what it calls "the Spherepop Calculus (SPC)" and includes an
  explicit `\begin{definition}[Spherepop event]`.

None of these four appear to define a *new* competing primitive
grammar (unlike families 4–7 above); they read as applications,
extensions, or worked examples built on top of an existing lineage
(most closely resembling family 1, the canonical four-primitive
model, based on the language quoted above). They should be treated as
supporting material for that lineage, pending a closer read, rather
than an eighth family.

**False positives — high-scoring files that are not Spherepop specs at
all**, despite the raw `Pop`/`Bind`/`Collapse` word-count being large:
- `textbook/persistence-before-truth.tex`, `textbook/
  negation-before-logic.tex`, `textbook/economy-of-forgotten-things.tex`,
  and `textbook/ecology-of-thought.tex` contain **zero** occurrences of
  the word "spherepop." Their high scores come entirely from ordinary
  English usage — "civilizations collapse," "error collapses,"
  "coordination structures collapse" — plus generic
  `definition`/`theorem` LaTeX environments unrelated to any Spherepop
  primitive.
- `textbook/fate-of-distinguishability.tex` contains exactly one
  "Spherepop" occurrence (a row label in what appears to be a
  comparison table). Its heavy `collapse`-related vocabulary
  (`collapse indicator`, `collapse stratum`, `fate space`) belongs to
  that document's own, unrelated "fate" framework — a false friend, not
  the Spherepop `Collapse` primitive.

**Lesson for future scans:** a keyword/structure score alone, without
gating on how often the document actually says "spherepop," reliably
promotes generic philosophy/math essays that happen to reuse common
English words (`pop`, `bind`, `collapse`, `refuse`) above genuine
Spherepop material. Every known genuine Spherepop document checked
during this session (`monograph/spherepop-calculus.tex`, `History as
Identity.tex`, `textbook/dependent-type-theory.tex`, `dynamics/
spherepop_scope_dynamics.tex`) mentions "spherepop" between 5 and 92
times — any high-scoring file with zero or a single incidental mention
should be treated as a likely false positive and hand-checked before
being added to this manifest, not trusted on score alone.

## Addendum: `experiments/flat/run_python.py` cross-checked against LINK/UNLINK/kernel MERGE/metadata/tensor/Branch/atomic composition

`run_python.py` (and the Rust kernel it conforms to) is a **JSON
fixture-op dispatcher**, not a textual-syntax parser — there is no
grammar/lexer anywhere in this repo's executable code; `events_from_op`
switches on a JSON `"op"` string. "Accepted syntax" below therefore
means "accepted as a JSON fixture `op`," the only notion of concrete
syntax this executable lineage has. Reference definitions are
`GRAMMAR.md` §8–9 (verified against `spherepop-os.tex`) and `GRAMMAR.md`
§ on the causal calculus's structural rules (sequence/tensor/Branch)
and interface mechanisms (atomic composition). Four statuses are used:
**(a) directly accepted**, **(b) fixture-level sugar** (expands to
primitive events), **(c) semantically specified but unavailable to the
parser/kernel**, **(d) wholly unimplemented**.

- **LINK / UNLINK — (b), and only a simplified proxy of the full OS
  semantics.** `link(a,b,tag)` → `[bind(a,b,tag)]` and `unlink(a,b)` →
  `[refuse(a,"relation withdrawn",b=b)]` (`run_python.py:188-193`),
  reachable via fixture ops `"link"`/`"unlink"` and exercised by
  `07_desugaring.json`. This matches `GRAMMAR.md`'s informal gloss
  ("Link(a,b) is literally Bind(a,b)... Unlink(a,b) is
  Refuse(Bind(a,b))") but is **not** the full OS-level UNLINK reduction
  that `GRAMMAR.md` §8–9 cites from `spherepop-os.tex`: `Collapse
  (Pop_revoke_r(Bind_f_valid(H_{a,b} ⊕ {revoke_r, retain_r})))`, which
  opens a **freshly opened option space** rather than emitting a single
  `Refuse`, per "Proposition UNLINK Does Not Erase." The flat fixture's
  single-`Refuse` proxy is a strictly weaker construct than the
  OS-level reduction it is named after — real but simplified.

- **kernel MERGE — (b), likewise simplified.** `merge(a,b,rule)` →
  `[bind(a,b,"merge"), collapse(rule)]` (`run_python.py:200-202`),
  fixture op `"merge"`, exercised by `06_derived_merge.json`. `GRAMMAR
  .md` §8–9/§412 describes the OS-level MERGE reduction as filtering
  *tensor-composed histories* for identity-compatible continuations and
  refusing the distinct alternative, **plus an external
  representative-selection policy requirement**. The fixture's two-event
  sugar implements neither the tensor-filtering step nor the explicit
  refusal of the rejected alternative nor a representative-selection
  policy — it is a minimal stand-in that produces an equivalence (via
  `collapse_quotient`'s union-find) without the richer OS-level
  justification.

- **metadata (`SET_META`) — (b) as implemented, but in apparent tension
  with the cited OS proof.** `set_meta(obj,key)` → `[bind(obj,key,
  "__meta__")]` (`run_python.py:204-206`), read back by
  `collapse_meta()` filtering on `tag == "__meta__"`; fixture op
  `"set_meta"`, exercised by `07_desugaring.json`. `GRAMMAR.md` §8–9
  explicitly cites `spherepop-os.tex`'s proof that "SET_META... admits
  no reduction to Pop/Refuse/Bind/Collapse/⊗ and sits outside the
  causal algebra entirely." The flat fixture suite's `set_meta` sugar
  does exactly what that proof says SET_META cannot do: reduce to a
  (tagged) `Bind`. This is not necessarily a bug — the flat suite is an
  intentionally narrow surface calculus, and a tagged-`Bind` encoding
  of metadata may be a deliberate, acknowledged simplification rather
  than a claim to have proven the OS theorem wrong — but it is a real,
  citable divergence between what the executable oracle does and what
  the cited specification proves, and should not be read as evidence
  that the OS proof is implemented or exercised anywhere.

- **tensor — (a), under the name "Meld," not the word "tensor."**
  `GRAMMAR.md` defines tensor as a structural rule combining two
  independently constructed histories, "not a fifth causal primitive."
  `run_python.py`'s `run_meld_fixture` (`run_python.py:237-256`)
  implements exactly this: two independent `Arbiter` histories are run
  separately, then concatenated (`histories[0] + histories[1]`).
  `08_meld.json`'s own `"invariant"` field names this "the free
  monoidal tensor of two independently-generated histories," and the
  fixture is executed and passes. The operation is real, tested, and
  matches `GRAMMAR.md`'s formal description precisely; only the literal
  JSON-level name (`history_a`/`history_b`/meld) differs from the
  calculus's word "tensor."

- **Branch — (c), wholly unimplemented, and this is a substantial
  gap.** `GRAMMAR.md` gives Branch a full typing judgment (`Γ⊢t⇒_e
  (t_p,t_c):Branch`), a shared-prefix coherence axiom (both projections
  `πp`/`πc` must share the exact history prefix at the branch event),
  and a fork/speculation identity-policy split. None of this appears
  anywhere in `run_python.py` or the Rust kernel it mirrors: there is
  no `branch` event kind, no dual-continuation state, no projection
  operators, and no identity-policy dispatch. The word "branch" occurs
  exactly once in the fixture corpus (`07_desugaring.json`, describing
  `Choice`'s refused option as "the untaken branch" — ordinary English,
  not the typed construct). Branch is genuinely specified in
  `GRAMMAR.md`/`spherepop-os.tex` but has no executable counterpart
  anywhere in this repository.

- **atomic composition — (c), and easy to mistake for something that
  is implemented.** `GRAMMAR.md` defines atomic composition, written
  `⟨⟨t_1;t_2⟩⟩`, as admitting several internal actions under one EID
  "so that no Collapse can observe an intermediate state" — an
  Arbiter/interface capability distinct from the causal or structural
  grammar. `Arbiter.submit` in `run_python.py` (`run_python.py:113-132`)
  does validate a whole batch of events before mutating history at all
  ("All-or-nothing: only mutate history after every event validates"),
  and `10_invalid_event.json` tests exactly this property. But this is
  a **strictly weaker** guarantee than `GRAMMAR.md`'s atomic
  composition: `submit` only prevents a *rejected* batch from partially
  mutating history; it does nothing to prevent an *intervening*
  `Collapse` from observing an intermediate state between two accepted,
  separately-submitted batches, and there is no EID concept in
  `run_python.py` at all. The genuine, specified atomic-composition
  construct (hiding intermediate states from Collapse under one EID) is
  unimplemented; what exists is a related but distinct transactional-
  validation property that happens to share the word "atomic."

**Summary table:**

| Construct | Status | Executable form (if any) |
|---|---|---|
| LINK / UNLINK | (b) simplified sugar | `link`/`unlink` → tagged `Bind` / single `Refuse` |
| kernel MERGE | (b) simplified sugar | `merge` → `Bind`+`Collapse`, no tensor-filter or rep-selection policy |
| metadata (SET_META) | (b) sugar, in tension with cited OS proof | `set_meta` → tagged `Bind`; OS proof says no such reduction exists |
| tensor | (a) accepted, under the name Meld | `run_meld_fixture` / `history_a`+`history_b` concatenation |
| Branch | (c) specified, unimplemented | none |
| atomic composition | (c) specified, unimplemented (a weaker relative exists) | `Arbiter.submit`'s all-or-nothing validation (not EID-scoped Collapse-hiding) |

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
   **not** linked against `spherepop_core`. All 13 fixtures present when
   this was last verified against the C/Rust binaries pass and agree
   with Rust and Python, clean under `-fsanitize=address,undefined`; the
   newer `13a`/`13b` quotient-equality pair (and `08_meld`'s executable
   schema from the same follow-up thread) have since been confirmed
   against Rust too, via a static-musl/`rust-lld` build that works
   around the missing system `cc` in this environment, but **not**
   against C — no C compiler of any kind (`cc`/`gcc`/`clang`/`tcc`/
   `zig`) is available here, so the C column for those three fixtures
   remains unverified. See `experiments/flat/CONFORMANCE.md`'s "Pending
   regeneration" notes and its live 3-way matrix.
   Closing the *interpreter's own* Bind/Collapse gap so that `.sp`
   source programs (not just the abstract fixture format) can express
   the canonical primitives remains open and is a separate, larger
   redesign task.
3. **Done this session — got SphereForth running via a bare-kernel
   gforth workaround, found and fixed seven genuine pre-existing bugs,
   and verified `demo.fs`/`REPLAY` end-to-end.** See the
   `prototypes/sphereforth_gforth.zip` entry above for the full list.
   Wiring it to the flat fixtures (`experiments/flat/`) remains open:
   its vocabulary is close but not identical to the canonical
   primitives (e.g. `COLLAPSE`'s synthesized `qN` token has no direct
   analogue in the JSON fixture schema), and the interactive `run` loop
   in `repl.fs` still couldn't be exercised here because the bare-kernel
   image's `refill` doesn't read piped stdin the way a full image would
   — a further environment limitation, not a code bug, worth re-checking
   in an environment with a real C toolchain (so the *normal* gforth
   startup path, not the bare-kernel workaround, can be used instead).
4. Fold the vocabulary false-friends found here (`reduction-engine`'s
   outcome-label `POP`/`COLLAPSE`; the Haskell/Racket calculus's
   unrelated `Pop`/`Merge`/`Choice`/`Rotate`) into the glossary work
   from Section 6 of the tracking issue, explicitly marking them as
   local synonyms with different meanings rather than leaving the
   overlap unexplained. **(Done — see `GLOSSARY.md`.)**
5. Start the Lean/formal-assurance track from zero rather than assuming
   any existing scaffolding — Phase F has no code basis yet.
