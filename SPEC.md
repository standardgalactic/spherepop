# The Spherepop Normative Specification (v1)

This is the short, normative semantic core called for by tracking issue
#1, Phase B ("Freeze a canonical semantic nucleus" / "Publish the
minimal event grammar and world model"). It is deliberately narrow:

- It defines only the four primitive events, the world they act on, the
  replay relation, the observation mechanism, well-formedness, errors,
  and equality of observable results.
- It is **not** a tutorial, a philosophical framework, a visual
  metaphor, or a domain-application claim. Those live in
  `Spherepop_Specifications.tex`, the monograph, `analysis/`,
  `projects/`, and the HTML/JS demos — see `GLOSSARY.md` for how each
  local vocabulary maps onto (or diverges from) the terms defined here.
- Every rule below is stated so that it is checkable against running
  code, not merely asserted. Each section cites the executable
  artifact(s) that already realize it: the Rust reference kernel
  (`spherepop-kernel/`), the independent Python oracle
  (`experiments/flat/run_python.py`), the independent C kernel
  (`compiler/tools/fixtures/`), and the flat fixture suite
  (`experiments/flat/fixtures/*.json`). Where this document and any of
  those three implementations disagree, the implementations are
  expected to be brought into line with this document (or this document
  amended under the versioning rule in §9) — not the other way around,
  since agreement between three independent implementations is the
  strongest evidence this project has that the semantics are stable.

This document supersedes, for the four primitives only, any
inconsistent older usage found elsewhere in the repository (see
`GLOSSARY.md`'s **false friend** entries for `Pop` and `Collapse` in
`reduction-engine/spherepop.py`; the unrelated term-calculus sense of
`Pop` in `spherepop.hs`/`spherepop.rkt` and in
`Spherepop_Specifications.tex`; and the **deviating**/**incomplete**
entries for `Bind` and `Collapse` in `compiler/`).

---

## 1. The world: `W = (H, Ω)`

A Spherepop world is a pair `W = (H, Ω)`:

- **`Ω` (the option space)** is a finite set of *object identifiers*
  (opaque values; the reference implementations use small non-negative
  integers, but nothing in this specification depends on that choice).
  `Ω` represents everything that remains possible: it starts as some
  initial `Ω₀` and can only ever *shrink* (via Pop), never grow, and is
  never itself the target of the append-only log.
- **`H` (the history)** is a finite, ordered sequence of *events*
  (§2), starting empty and only ever growing by appending to its end.
  `H` is the *sole* authoritative record; everything else in this
  specification — the current `Ω`, what has been refused, what is
  bound, what has been observed — is a value *derived* from `H` by
  **replay** (§4), never stored or mutated independently of it.

`H` is a free monoid under concatenation (empty history = identity,
append = the monoid operation); `Ω` is a plain finite set with one
allowed operation (remove-one-element, via Pop).

*Executable correspondence:* `spherepop-kernel::History` /
`spherepop-kernel::Arbiter` (`spherepop-kernel/src/history.rs`,
`arbiter.rs`); `experiments/flat/run_python.py`'s `Arbiter`/`State`
classes; `compiler/tools/fixtures/kernel.h`'s `Arbiter`/`State` structs.

---

## 2. The event grammar

There are, and only ever will be, **four primitive event kinds**. No
other event kind exists in this layer, no matter how many convenience
forms are built on top of them (§7):

| Event | Fields | Canonical meaning |
|---|---|---|
| **`Pop(a)`** | `a: ObjectId` | Commit to option `a`, removing it from `Ω`. Irreversible. |
| **`Refuse(a, reason[, b])`** | `a: ObjectId`, `reason: String` (required, non-empty), `b: ObjectId` (optional) | Record that `a` (or, with `b` present, the pair `(a, b)`) is inadmissible, *without* removing anything from `Ω`. |
| **`Bind(a, b, tag)`** | `a, b: ObjectId`, `tag: String` (may be empty) | Couple `a` and `b` as dependent, tagged with a relation label. Never identifies `a` and `b` — they remain distinct members of `Ω`/`committed`. |
| **`Collapse(rule)`** | `rule: RuleId` (a certified name, see §4) | Observe `H` under the named rule, projecting it onto that rule's quotient space, without mutating `H` or any derived state. |

An event's fields beyond its kind are **additive and kind-dependent**:
a decoder must treat a field that is absent/inapplicable for a given
kind as "not applicable," never as malformed data. (Concretely: `Bind`
never reads `reason` or `rule`; `Collapse` never reads `a`/`b`/`tag`;
etc.) This is an explicit forward-compatibility rule, not an accident
of the current encoding — see §9.

*Executable correspondence:* `spherepop-kernel::EventKind` /
`Event` (`event.rs`); `run_python.py`'s `Event` dataclass;
`compiler/tools/fixtures/kernel.h`'s `Event` struct/`EventKind` enum.
All three independently define exactly these four kinds and nothing
else — confirmed by the fact that `experiments/flat/fixtures/*.json`
uses only these four event kinds (plus the derived sugar ops of §7,
which *expand into* them before reaching the Arbiter) and all three
implementations agree on every fixture (`experiments/flat/CONFORMANCE.md`).

---

## 3. Well-formedness

An event, or a batch of events submitted together (a *proposal*), is
**well-formed** with respect to a world `W = (H, Ω)` iff:

1. **`Pop(a)` is well-formed** iff `a ∈ Ω` and `a` is not already
   committed by an earlier event in the *same* proposal (a proposal may
   not pop the same object twice, even before either has been appended
   to `H`).
2. **`Refuse(a, reason[, b])` is well-formed** iff `reason` is
   non-empty. (Refusing without a reason documents nothing and is
   rejected outright — a refusal that cannot say *why* is not a
   refusal.) There is **no** requirement that `a` (or `b`) be in `Ω` —
   an object may be refused whether or not it was ever committable,
   and whether or not it was already committed.
3. **`Bind(a, b, tag)` is well-formed** unconditionally over `a`/`b`
   (they need not be in `Ω`, need not be uncommitted, and need not be
   distinct from objects appearing in earlier Binds). `Bind` records a
   relation; it does not gate on option-space membership because
   coupling two objects is not itself a commitment.
4. **`Collapse(rule)` is well-formed** iff `rule` is a member of the
   world's *certified rule set* (a fixed set of rule names fixed at
   world-creation time — see §5). An uncertified rule name is rejected
   as a type error at proposal time, precisely so that "what a program
   is allowed to observe" is itself part of the world's static
   configuration, not something a running program can expand on the
   fly.

A **proposal** (a list of one or more events submitted together) is
well-formed iff every event in it is well-formed *given the effect of
the events before it in the same proposal* (i.e. well-formedness is
checked against the hypothetical state obtained by folding the
proposal's own earlier events on top of the world's current state —
this is what makes rule 1 above a same-proposal, not just a
same-history, check).

**Atomicity:** a well-formed proposal's events are appended to `H` as a
single, indivisible step. An **ill-formed** proposal is rejected in its
entirety and `H` is left byte-for-byte unchanged — there is no partial
application of a rejected proposal.

*Executable correspondence:* `spherepop-kernel::Arbiter::validate`/
`submit` (`arbiter.rs`); `run_python.py`'s `Arbiter.submit`;
`compiler/tools/fixtures/kernel.c`'s `arbiter_submit`. Fixture
`10_invalid_event.json` is the checked-in negative case for rule 1 and
the atomicity guarantee together.

---

## 4. The transition / replay relation

The **state** `S` derivable from a world at any point is the 5-tuple:

```
S = (option_space, committed, bound, refused, observed)
```

where:

- `option_space ⊆ Ω₀` — the objects not yet Popped.
- `committed ⊆ Ω₀` — the objects already Popped (disjoint complement of
  `option_space` within `Ω₀`, by construction — nothing else changes
  `option_space` or `committed`).
- `bound ⊆ Ω₀ × Ω₀ × Tag` — the set of `(a, b, tag)` triples recorded
  by every `Bind` event so far. (A set, not a sequence: two identical
  `Bind` events contribute one element, though the underlying `H` still
  records both occurrences — see §6's note on `bound` vs. `H` itself.)
- `refused` — the ordered list of `(position, target, reason)` triples,
  one per `Refuse` event, in the order they were appended.
- `observed` — the ordered list of `(position, rule)` pairs, one per
  `Collapse` event, in the order they were appended. Note precisely
  what is **not** here: the *value* a collapse rule computes (`c(H)`,
  e.g. which objects a quotient rule identifies) is never stored in
  `S` itself — only the fact that rule `c` was invoked, and when. The
  value itself is always recomputed on demand by applying the named
  rule as a pure function of `H` (§5). This is **Observation
  Non-Interference**: there is no field of `S`, and no parameter to the
  well-formedness check in §3, through which any collapse rule's
  output could ever feed back into what future proposals are accepted.

The pure, per-event transition function `apply : (S, Event) → S` is
defined by cases exactly as follows (nothing else changes any field of
`S`, including the case not listed, which does not exist):

| Event | Effect on `S` |
|---|---|
| `Pop(a)` | `option_space := option_space \ {a}`; `committed := committed ∪ {a}` |
| `Refuse(a, reason[, b])` | `refused := refused ++ [(pos, a, reason)]`; **nothing else changes** — in particular `option_space` is untouched |
| `Bind(a, b, tag)` | `bound := bound ∪ {(a, b, tag)}` |
| `Collapse(rule)` | `observed := observed ++ [(pos, rule)]`; **nothing else changes** |

**Replay** is the fold of `apply` over `H` starting from
`S₀ = (Ω₀, ∅, ∅, [], [])`:

```
replay(H, Ω₀) = fold(apply, S₀, H)
```

Because `apply` is a pure function of `(S, Event)` alone — no
wall-clock reads, no ambient/global state, no dependency on how many
times replay has previously run — **Deterministic Replay** holds by
construction, not merely by testing: `replay(H, Ω₀)` called twice on
the same `(H, Ω₀)` always yields two values equal under the equality
relation of §6.

*Executable correspondence:* `spherepop-kernel::apply`/
`History::replay` (`history.rs`); `run_python.py`'s module-level
`apply` function and `Arbiter.state()`; `compiler/tools/fixtures/
kernel.c`'s `state_apply`/`arbiter_state`. The Rust crate's own test
suite (`spherepop-kernel/src/lib.rs`) includes a test named literally
`deterministic_replay`, and fixture `09_replay.json` checks the same
property via the `"deterministic_replay": true` expectation field,
passing identically in all three implementations.

---

## 5. Collapse rules

A **collapse rule** `c` is a pure function `c : H → O_c` for some
observation space `O_c` — it reads the full event history (not just
the derived state `S`) and produces an external observation value,
without ever mutating `H` or `S`. Which named rules exist, and what
each computes, is **not fixed by this specification** as a closed list
— rules are named, pluggable, and a world is parameterized by a fixed
*certified rule set* (the names it will accept in a `Collapse` event;
see §3, rule 4). This specification does, however, fix four reference
rules that every conforming implementation is expected to provide
identically, because the flat fixture suite exercises them by name:

| Rule name | `O_c` | Definition |
|---|---|---|
| `identity` | the full event sequence | `c(H) = H` itself — the finest possible observation, nothing quotiented. |
| `quotient` | equivalence classes over `ObjectId` | Union-find over every `Bind(a, b, tag)` with `tag ≠ "__meta__"`: `a` and `b` become same-class. This rule **is** Merge — see §7. |
| (quotient honoring refusals) | equivalence classes over `ObjectId` | As `quotient`, but a `Bind(a,b,_)` is excluded from unioning if a later `Refuse(a, "relation withdrawn", b)` (or the `(b,a)` order) exists anywhere in `H`. Demonstrates that a rule *may* choose to respect a documented withdrawal — this is the rule's own policy, not a structural deletion from `H`, which never happens. |
| `__meta__` reader | `ObjectId → List[ObjectId]` | Reads every `Bind(a, b, "__meta__")` as "object `a` carries metadata key/value `b`," distinguishing it from ordinary relations. Ordinary rules (`quotient` and its refusal-honoring variant) are specifically defined to skip `"__meta__"`-tagged binds. |

A rule name used in a `Collapse` event is a label chosen by the
proposer; the world's certified-rule-set check (§3) only verifies the
*name* is one this world accepts, never the *value* the rule would
compute — accepting a `Collapse` event is a decision made purely from
`(H, Ω)`-level facts, never from `c(H)` for any `c` (this is the same
Observation Non-Interference property from §4, restated as a
constraint on well-formedness rather than on state).

*Executable correspondence:* `spherepop-kernel::collapse::{
collapse_identity, collapse_quotient, collapse_quotient_honoring_refusals,
collapse_meta}` (`collapse.rs`); `run_python.py`'s
`collapse_quotient`/`collapse_quotient_honoring_refusals`/
`collapse_meta`; `compiler/tools/fixtures/collapse_rules.c`'s
`collapse_quotient`/`collapse_quotient_honoring_refusals`/
`collapse_meta_has_key`. Fixtures `04_collapse_as_observation.json`,
`06_derived_merge.json`, and `07_desugaring.json` exercise these by
name across all three implementations.

---

## 6. Equality of observable results

Two worlds' derived states `S₁`, `S₂` are **observationally equal**
iff, componentwise:

- `option_space` and `committed` are equal as sets;
- `bound` is equal as a set of `(a, b, tag)` triples (not a sequence —
  see §4's note);
- `refused` and `observed` are equal as *sequences*, since their
  ordering is itself part of what they record (position matters: two
  histories that refuse the same objects in a different order, or with
  different stated reasons, are not observationally equal even though
  their `option_space` might coincide);
- for every certified rule `c`, `c(H₁) = c(H₂)` under `O_c`'s own
  equality (e.g. two union-finds are equal iff they induce the same
  partition, regardless of internal representation).

**A terminal `option_space`/`committed` snapshot alone is *not*
sufficient** to establish observational equality — this is precisely
what fixtures `05a_same_snapshot_different_history_run_a.json` and
`05b_same_snapshot_different_history_run_b.json` demonstrate: two runs
reaching an identical terminal `Ω` via different `bound`/`refused`
provenance are *not* observationally equal, and any implementation
that only tracked the terminal snapshot could not tell them apart.

*Executable correspondence:* `spherepop-kernel::State`'s derived
`PartialEq` (`history.rs`); `run_python.py`'s `State` dataclass
equality (used directly by the `deterministic_replay` check);
`compiler/tools/fixtures/kernel.c`'s `state_equals`.

---

## 7. Primitive / derived / structural boundary

Only the four events of §2 are **primitive** — they are the only
`EventKind`/`op` values the Arbiter itself understands, and the only
ones that ever reach `apply`. Everything else in this specification is
one of two other things, and every other name a program-facing surface
might use is required to reduce to one of them with **no new event
kind introduced**:

**Derived (surface-calculus sugar):** a fixed expansion into a short
sequence of primitive events, submitted and validated exactly as if a
caller had written the primitive events directly. This document fixes
five such forms (others may be added under the additive-only rule of
§9, but must likewise expand to primitives only):

| Derived form | Expands to |
|---|---|
| `Link(a, b, tag)` | `Bind(a, b, tag)` — literally, no other event. |
| `Unlink(a, b)` | `Refuse(a, "relation withdrawn", b)` — documents withdrawal; does **not** delete or alter the original `Bind(a, b, _)` event, which remains in `H` unchanged. |
| `Choice(taken, rejected)` | `Pop(taken)`, `Refuse(rejected, "not selected by Choice")` — committing to one option while auditably refusing the other, rather than letting the untaken branch silently vanish. |
| `Merge_c(a, b)` | `Bind(a, b, "merge")`, `Collapse(c)` — "merging" is *exactly* a Bind immediately followed by an observation under an identifying quotient rule; there is no separate merge mechanism. |
| `SetMeta(object, key)` | `Bind(object, key, "__meta__")` — metadata is an ordinary Bind under a reserved tag that the `quotient` family of rules is specifically defined to ignore (§5). |

Sphere and Nest (from the wider Spherepop vocabulary — see
`GLOSSARY.md`) are likewise derived, expanding to chains of `Bind`/
`Collapse` pairs, but are not yet given executable adapters in this
kernel; they are flagged `unresolved` in `GLOSSARY.md` rather than
silently assumed complete.

**Structural (composition of histories themselves):** **Meld** is the
free monoidal tensor of two independently-generated histories —
concatenation of two event logs that were never both under one
Arbiter's option-space accounting at the same time. It is categorically
different from `Bind`-based concurrency (which couples elements
*within* a single shared history) and is **not** a fifth primitive
event: it operates one level up, on `History` values themselves, before
either is submitted to an Arbiter. `Meld` has its own two-history
fixture schema (`08_meld.json`'s `history_a`/`history_b`/
`expect_melded_history_len` fields, distinct from the single-Arbiter
`initial_option_space`/`events`/`expect` schema the other fixtures use)
— see that fixture and `IMPLEMENTATIONS.md`/`CONFORMANCE.md` for its
current status (executed end-to-end by all three fixture runners).

*Executable correspondence:* `spherepop-kernel::sugar::{link, unlink,
choice, merge, set_meta}` (`sugar.rs`) and `History::meld`
(`history.rs`); `run_python.py`'s identically-named functions;
`compiler/tools/fixtures/sugar.c`. Fixture `07_desugaring.json` checks,
by name, that every derived form above expands to only `Pop`/`Refuse`/
`Bind`/`Collapse` events and nothing else, across all three
implementations.

---

## 8. Errors and refusals

**Refusal is data, not failure.** A well-formed `Refuse` event is
accepted, appended to `H`, and changes `S.refused` — it is a normal,
successful step in a history, and is the mechanism by which a program
records that something is inadmissible without erasing it from `Ω` or
from the record. Do not conflate this with rejection.

**Rejection** is what happens to an ill-formed proposal (§3): it is
refused *entry to `H` at all*, atomically, and is reported as one of a
fixed set of errors:

| Error | Raised when |
|---|---|
| `Malformed(reason)` | An event is missing a field its kind requires (e.g. `Bind` without both `a` and `b`, `Collapse` without a `rule`). |
| `PopOutsideOptionSpace(a)` | A `Pop(a)` targets an `a` not currently in `Ω` (already committed, never in `Ω₀`, or committed earlier in the same proposal). |
| `RefuseWithoutReason` | A `Refuse` event's `reason` is empty. |
| `UncertifiedCollapseRule(rule)` | A `Collapse` event names a rule not in the world's certified rule set. |
| `StaleOverlay` | (Overlay-manager use only, §9-adjacent — see `overlay.rs`.) A speculative proposal is committed after `H` has grown past the point at which it was previewed. |

A rejected proposal leaves `H`, and therefore every derived field of
`S`, byte-for-byte unchanged — this is checked directly by fixture
`10_invalid_event.json`'s `history_len` expectation after a rejected
second `Pop`.

*Executable correspondence:* `spherepop-kernel::ArbiterError`
(`arbiter.rs`); `run_python.py`'s `ArbiterError` exception and its
message strings; `compiler/tools/fixtures/kernel.c`'s `arbiter_submit`
error strings (`PopOutsideOptionSpace(...)`, `RefuseWithoutReason`,
etc., matched by prefix in fixture `10`'s `expect_error` field).

---

## 9. Serialization and canonical output

The only serialization format specified *and exercised* as of this
version is the flat fixture JSON format in
`experiments/flat/fixtures/*.json`: an object with
`initial_option_space` (array of object ids), `certified_rules` (array
of rule names), `events` (array of `{"op": ..., ...}` objects, one of
the primitive or derived op names from §2/§7), and an `expect` block
whose fields correspond directly to the components of `S` in §4 and
the rules of §5 (`option_space`, `committed`, `bound`, `refused_count`,
`observed_rules`, `history_len`, `quotient_same_class`,
`quotient_honoring_refusals_same_class`, `meta_keys`,
`deterministic_replay`). Its fields are additive in the same sense as
an `Event`'s (§2): a fixture may omit any `expect` field it does not
need checked, and a checker must not treat an absent field as a
zero/false expectation.

The canonical wire encoding of a replayable world is **SPHIST/1**, defined
byte-for-byte in `experiments/flat/HISTORY-WIRE-V1.md`. It contains the
sorted initial option space, sorted certified-rule names, and ordered
primitive history. Event positions are implicit in sequence order, and
derived forms must be desugared before encoding. It deliberately excludes
terminal state and Collapse outputs: those remain replay products.

Fixture `09_replay.json` fixes one golden 107-byte envelope and its
FNV-1a-64 digest. Each conforming adapter constructs the envelope from its
executed history, checks the digest, decodes without consulting the fixture
event list, and replays from a fresh state initialized only from the
decoded option space. FNV-1a-64 is a stable conformance identifier, not a
cryptographic authentication mechanism.

---

## 10. Versioning rules

This specification is versioned independently of any single
implementation. Until a formal version-numbering scheme is adopted,
the following rules govern any future change to this document or to
the event grammar it defines:

1. **The four primitive event kinds (§2) are closed.** A future version
   may add derived (§7) forms or additional certified collapse rules
   (§5) freely — that is an additive, backward-compatible change. It
   may **not** add a fifth primitive `EventKind`, rename an existing
   one, or change an existing primitive's core effect on `S` (§4's
   table) without incrementing a major version and explicitly
   documenting the break, including updating every fixture and
   implementation that depended on the old behavior.
2. **Fields are additive-only.** A new optional field may be added to
   the event or fixture-expectation schema at any time; an existing
   field may not be removed or repurposed to mean something different
   for the same kind. This is the same rule already stated as an ABI
   note in §2 and applied consistently to the fixture format in §9.
3. **New certified collapse rules do not require a version bump**, since
   a world's certified rule set is already part of that world's own
   configuration (§3, §5) — adding a rule name a *particular* world
   accepts is a local decision, not a change to this specification.
   Changing what an *existing, named* rule (`identity`, `quotient`,
   the refusal-honoring variant, or the `__meta__` reader) computes,
   however, **is** a semantic break and requires the same major-version
   treatment as primitive changes.
4. **Errors (§8) are closed under the same rule as primitives**: the
   fixed error set may grow (e.g. to support future overlay or
   serialization features) but an existing error's trigger condition
   may not silently change.
5. Any version bump under this section must be accompanied by: an
   entry in this section's changelog (below), a corresponding update to
   `experiments/flat/CONFORMANCE.md`'s adapters (a version mismatch
   between an implementation and the fixtures it runs against should be
   visible as a failing conformance row, not a silent pass), and an
   entry in `GLOSSARY.md` if any term's status changes.

### Changelog

- **v1** (this document, first published version) — codifies the
  four-primitive event grammar, world model, well-formedness, replay,
  four reference collapse rules, five derived sugar forms, Meld as a
  structural (non-primitive) operation, the fixed error set, and the
  SPHIST/1 serialization and canonical digest of §9, as realized identically
  by `spherepop-kernel/`, `experiments/flat/run_python.py`, and
  `compiler/tools/fixtures/`, and checked by every fixture in
  `experiments/flat/fixtures/`.

---

## 11. What this document is not

This specification states definitions, well-formedness rules, and a
replay/observation relation, and cites where each is already realized
by running code and checked by fixtures. It does **not** itself
constitute:

- A **proof** of any property stated as holding "by construction"
  above (e.g. Deterministic Replay, Observation Non-Interference,
  Irreversibility/History-only-grows, Conservation of Possibility —
  the last three are named and unit-tested in
  `spherepop-kernel/src/lib.rs` as `irreversibility_history_only_grows`,
  `conservation_of_possibility`, and
  `observation_cannot_influence_later_acceptance`, but a unit test
  checking finitely many cases is evidence, not a formal proof for all
  histories). Stating and mechanically checking these as theorems is
  Phase F ("Formal assurance") work and is tracked as not yet started
  in `IMPLEMENTATIONS.md`.
- A claim about what programs Spherepop can or cannot express in
  general (Phase E, "Language adequacy," full scope — translations
  from a known small calculus, still open). An initial, narrower
  instance of this evidence level — a tiny lambda-calculus fragment
  translated into primitive-event traces, checked against an
  independent reference evaluator across seven example programs — is
  in `experiments/adequacy/` (see its `README.md` for exactly what is
  and is not claimed).
- A history of how these definitions were arrived at (see the planned
  Phase A internal-history deliverable) or a justification of why this
  particular basis was chosen over an alternative one (see the
  monograph and `Spherepop_Specifications.tex` for that argument).
- A statement about any implementation not listed in this document's
  opening citation list; `compiler/`'s general-purpose Bubble
  interpreter, in particular, is **explicitly out of scope** here since
  its own `Bind`/`Collapse` are documented (in `IMPLEMENTATIONS.md`) to
  diverge from §2/§4/§5 above.
