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
