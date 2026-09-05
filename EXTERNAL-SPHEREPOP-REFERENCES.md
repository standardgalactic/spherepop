# External Spherepop references — cross-repository inventory

**Status:** All 14 listed `.tex` documents (16 counting exact duplicates),
all named/located Markdown/`.txt` essays, and the 3 duplicate pairs
(6 paths) noted below have now been fetched and read in full; each is
classified as canonical use, non-canonical-but-historically-relevant
Spherepop-branded lineage, incidental mention, or exact duplicate. No
files remain unverified. This document records a cross-repository
search the user performed (via a separate reporting tool, "the Library
skill") across their other GitHub repositories, excluding
`standardgalactic/spherepop` itself and `standardgalactic/personalinks`
(already covered in `GRAMMAR.md` and `IMPLEMENTATIONS.md`). The full source
report (`spherepop-cross-repository-report.md`) exists outside this repo and
outside this session's filesystem; the user pasted its contents into this
session, and this document is a checked, repo-resident summary of that
report — not the report itself.

## What the source report claims

Searching the default branches of `standardgalactic`-owned repositories
(excluding `standardgalactic/spherepop`) for "Spherepop" in `.tex`, `.md`,
and `.txt` files found:

- **133 verified matching file paths across 14 repositories**
- **122 distinct file contents** after collapsing exact (Git-blob-SHA)
  duplicates
- **16 actual `.tex` paths, representing 14 distinct LaTeX documents**
- The strongest independent formal concentrations: `research-projects`,
  `alphabet`, `calculus`, `computation`, `laboratory`
- `kitbash`'s 35 matches are predominantly archival snapshots (files named
  like `archive/Joy-of-Spherepop-draft-01.tex.txt`), not independent essays
- **Known limitation the source report itself states:** GitHub code search
  caps results at 100 per query, so this is a verified account of every
  *surfaced* candidate, not a mathematical guarantee of exhaustiveness. A
  fully exhaustive account would require cloning every repository (all
  branches) and running an offline query locally — e.g. an adaptation of
  this repo's own `scripts/tex-spec-query.sh`.

## The 14 distinct `.tex` documents claimed

| Repository | File | Claimed character | Mentions | Formal signals |
|---|---|---|---:|---:|
| `alphabet` | `aphantasia/quantum-spherepop.tex` | formal LaTeX work | 25 | 102 |
| `alphabet` | `roadmap/flyxion_briefing.tex` | formal LaTeX work | 28 | 38 |
| `alphabet` | `working/drafts/function_survives_collapse.tex` | formal LaTeX work | 7 | 43 |
| `alphabet` | `working/function_survives_collapse.tex` | formal LaTeX work | 14 | 58 |
| `calculus` | `identity_after_collapse.tex` | formal LaTeX work | 122 | 121 |
| `calculus` | `working/Identity_After_Collapse.tex` | formal LaTeX work (byte-identical to the above) | 122 | 121 |
| `computation` | `collapse_retention.tex` | LaTeX essay | 6 | 1 |
| `computation` | `processing/collapse_retention.tex` | LaTeX essay (byte-identical to the above) | 6 | 1 |
| `laboratory` | `working/spherepop-os.tex` | formal LaTeX work (working copy) | 16 | 243 |
| `research-projects` | `polanyi-spherepop.tex` | LaTeX essay | 4 | 14 |
| `research-projects` | `prefigurative-spherepop.tex` | LaTeX essay | 3 | 14 |
| `research-projects` | `RSVP-Monograph/.../ch01-spherepop-homotopy.tex` | LaTeX essay | 12 | 7 |
| `research-projects` | `RSVP-Monograph/.../ch02-spherepop-truncation.tex` | LaTeX essay | 3 | 4 |
| `research-projects` | `RSVP-Monograph/.../ch03-spherepop-computation.tex` | LaTeX essay | 18 | 7 |
| `research-projects` | `spherepop-calculus.tex` | formal LaTeX work | 5 | 27 |
| `research-projects` | `spherepop-derivation.tex` | LaTeX essay | 5 | 14 |

## A methodological caveat, confirmed by this verification pass

**Important: the table above shows the source report's own scoring
("Claimed character" / "Formal signals"), not a verified classification.**
That score measures the presence of formal-looking vocabulary and LaTeX
structures (`\begin{definition}`, `\begin{theorem}`, dense operator
notation, etc.) — it does **not** measure whether those structures
actually specify Spherepop. Verifying three of the fourteen documents
directly against their source produced one confirmed true positive, one
confirmed accurate non-independence classification, and one confirmed
**false positive** — proving the same lesson already recorded in
`IMPLEMENTATIONS.md`'s addendum for `tex-spec-query.sh`: keyword/structure
scoring is useful for *candidate discovery* (surfacing files worth
checking) but cannot by itself establish *classification* (whether a file
is actually a Spherepop calculus, grammar, or implementation
specification). Every entry below is therefore split into "automatically
surfaced as potentially formal" (the source report's score) versus
"verified by source inspection" (this session's direct reading).

## Verification performed in this session

Three of the fourteen documents were fetched directly via the GitHub API
and read in full. Their verified classification:

1. **`calculus/identity_after_collapse.tex` — verified: a genuine
   executable-style formal calculus using the four primitive names.**
   Defines real macros `\pop`, `\refuse`, `\bind`, `\collapseop` and
   states evaluation is "a one-way collapse of nested structure into a
   terminal trace." Its **bubble semantics diverge from the canonical
   option-space/history kernel** (`SPEC.md`) — this is an independent
   calculus that happens to share the same four primitive names, not a
   duplicate or a conforming implementation of the canonical model. Also
   confirmed: `calculus/identity_after_collapse.tex` and
   `calculus/working/Identity_After_Collapse.tex` share the identical Git
   blob SHA (`7389225cedb3227319f7b390d67e71f4896cbf70`) — genuinely
   byte-identical, as the source report claims.
2. **`laboratory/working/spherepop-os.tex` — verified: an exact duplicate
   of this repository's own `spherepop-os.tex`, not an independent
   specification.** 1291 lines, matching line-for-line. The source
   report's "working specification copy" label is directionally right but
   understates it: this is not merely *a* working copy of *an* OS
   specification, it is a copy of the exact same document already sitting
   in `standardgalactic/spherepop`'s repository root.
3. **`alphabet/aphantasia/quantum-spherepop.tex` — verified: an
   incidental mention, not a Spherepop calculus, grammar, implementation
   specification, or branded lineage.** Despite being scored "formal
   LaTeX work" with 102 "formal signals," it is a domain-level
   quantum-physics paper on the "Relativistic Scalar-Vector Plenum
   (RSVP)" that uses "Spherepop" only as a one-off metaphor — e.g. "local
   'pops' correspond to amplitude relaxations," "their 'popping'
   corresponds to phase collapse or relaxation of local inconsistency."
   There is no `Refuse` or `Bind` primitive, no event-log/history model,
   and critically **no self-contained named operator system of its own**
   (unlike the branded lineages classified below) — the high score
   reflects ordinary physics-paper apparatus (`\begin{definition}`,
   `\begin{theorem}`, `\begin{proposition}`) entirely unrelated to
   Spherepop's grammar. **This file must not be counted toward the
   14-distinct-LaTeX-document total as a specification, nor as a distinct
   branded lineage** — it is a domain application referencing Spherepop
   by name and metaphor only, with no accompanying formalism to classify
   as a lineage.

The remaining 11 documents in the table above have **not** been
independently verified in this session — they are "automatically surfaced
as potentially formal" only, and must be read directly before being
treated as verified specifications, per the same classification risk
demonstrated above: keyword/structure scoring cannot distinguish
canonical use, non-canonical branded lineages, and incidental mentions
from one another.

## Verification of the remaining 11 documents (later session)

All 11 previously-unread documents (plus one, `research-projects/spherepop-
derivation.tex`, that had been requested but not yet returned) were fetched
and read in full via the GitHub API. Findings:

4. **`computation/collapse_retention.tex` and
   `computation/processing/collapse_retention.tex` — verified: genuine,
   byte-identical (same Git blob SHA), and a real extension of the
   canonical calculus.** "What Collapse Should Forget: A Retention
   Calculus for Spherepop" explicitly builds on "the earlier essay's
   typing rules" for `Pop`/`Refuse`/`Bind` and adds real formal content
   (sufficient statistics, rate-distortion theory, information
   bottleneck) to the canonical `Collapse` operator. Unlike
   `quantum-spherepop.tex`, this is not a metaphorical reuse — it
   directly extends the four-primitive vocabulary with a new technical
   proposal for *what* `Collapse` should retain/forget. The source
   report's "LaTeX essay" label is accurate but understates it: this is
   closer to a genuine, if speculative, addition to the canonical
   calculus's `Collapse` semantics, not merely an essay about it.

5. **`research-projects/polanyi-spherepop.tex`,
   `research-projects/prefigurative-spherepop.tex`,
   `research-projects/spherepop-calculus.tex`, and
   `research-projects/spherepop-derivation.tex` — verified: a
   non-canonical, but historically relevant, "Spherepop"-branded
   lineage** (the "Flyxion/RSVP SPC" lineage — see also item 9 and the
   `The Geometry of Spherepop.txt` duplicate below, which belong to the
   same family), distinct from `SPEC.md`'s canonical calculus and from
   `calculus/identity_after_collapse.tex`'s independent bubble calculus.
   All four are by the same author group (Flyxion, with Grok and an
   "Anonymous Playcosm Author" as co-authors on at least one), and all
   four use a *single* operator called `pop`/`\text{pop}` merging two
   "spheres" formalized as probability-space/entropy triples
   $(I, B, \Sigma)$ — identity, boundary, entropy — under
   interface-compatibility, cost, and entropy criteria. There is **no
   option space, no append-only history, no `Refuse`/`Bind`/`Collapse`
   quartet, and no Arbiter/admissibility model** anywhere in these four
   documents. `spherepop-derivation.tex` and `spherepop-calculus.tex`
   integrate this single-`pop` formalism with Ellul's *The Technological
   Society* and a "Playcosm" single-shard-universe framework (privilege
   gates as "pop regimes," prefigurative play as "anti-admissible
   spheres"). **These four documents must not be counted as candidate
   specifications of the canonical Spherepop calculus**, but they should
   be retained as evidence of genuine branching/semantic reuse of the
   name across a real, internally consistent alternative formalism —
   distinguishing this **non-canonical lineage** from an **incidental
   mention** like `quantum-spherepop.tex` (item 3 above), which has no
   accompanying formalism of its own.

6. **`alphabet/roadmap/flyxion_briefing.tex` — verified: a genuine,
   accurate high-level survey description of the canonical four-primitive
   calculus**, not a new or divergent specification. Its "Historical
   Computation: MEM|8 and Spherepop" chapter describes `Pop`, `Refuse`,
   `Collapse`, and `Bind` in terms consistent with the canonical model
   (e.g. "Refuse records the rejection... making the rejection itself
   part of the history," "Collapse produces a compressed state summary...
   at a level of detail specified by the calling context," "Bind creates
   a named reference to a history"), and states "the current
   implementation is a C interpreter with event-sourced semantics... A
   Rust implementation with PyO3 bindings is in development." This is
   survey/briefing-level prose, not itself a formal spec, but its
   description of the primitives does not contradict the canonical
   model — unverified against this repository is whether a C interpreter
   or Rust/PyO3 bindings actually exist anywhere in
   `standardgalactic/spherepop`'s own history (no such artifacts are
   present in this repository as of this session).

7. **`alphabet/working/drafts/function_survives_collapse.tex` (7
   mentions) and `alphabet/working/function_survives_collapse.tex` (14
   mentions) — verified: genuine, consistent references to Spherepop as
   one of four sibling frameworks in a larger "RSVP, Spherepop,
   Yarncrawler, Adm" theoretical stack**, not a redefinition. Both
   describe "the Pop operator collapses a bubble into its projection
   class, the Bind operator establishes a new equivalence relation, the
   Collapse operator finalizes the quotient, and the Refuse operator
   prevents certain transitions" — genuinely all four canonical primitive
   names, used with roughly consistent semantics, inside a "bubble
   calculus" framing matching `calculus/identity_after_collapse.tex`'s own
   bubble-based `pop`/`refuse`/`bind`/`collapseop` (same author family).
   The two files are **not byte-identical** — the non-draft version has
   twice as many mentions (14 vs 7), including an additional ~7-mention
   section ("Speech as Programmable Infrastructure and the Spherepop
   Connection," discussing a song lyric "Spherepop is a system, we write
   the future with sound") absent from the draft — confirming these are
   two genuinely different revisions, not duplicates, despite the source
   report grouping them together in the same repository.

8. **`calculus/working/Identity_After_Collapse.tex` — verified: byte-
   identical to `calculus/identity_after_collapse.tex`**, confirmed via
   matching Git blob SHA (`7389225cedb3227319f7b390d67e71f4896cbf70`),
   matching item 1 above exactly. No new classification needed — this is
   the same document already counted once.

9. **The three `research-projects/RSVP-Monograph/Volume-III-Extensions/
   chapters/ch01-spherepop-homotopy.tex`, `ch02-spherepop-truncation.tex`,
   `ch03-spherepop-computation.tex` — verified: a fifth, self-contained,
   non-canonical "SpherePop" formalism** (capitalized differently:
   "SpherePop"), citing a separate prior work `Flyxion-SpherePop-v1`. This
   formalism has **only two primitives, `merge` and `collapse`**, defined
   over configuration spaces of embedded $n$-spheres in a manifold
   enriched with "lamphrodynamic fields" $(\Phi,v,S)$ tied to the RSVP
   framework. Chapter 1 models configuration spaces as $\infty$-groupoids
   and merge as a homotopy colimit; Chapter 2 conjectures collapse is a
   derived ($\infty$-categorical) truncation; Chapter 3 claims (with only
   proof sketches) that merge+collapse together encode untyped lambda
   calculus and are therefore Turing complete. **There is no `Pop`,
   `Refuse`, or `Bind` primitive, no option space, and no event history**
   anywhere in these three chapters — this is unrelated to the canonical
   calculus beyond sharing the name and the general idea of a
   "collapse"-like abstraction operator. **These three chapters must not
   be counted as candidate specifications of the canonical Spherepop
   calculus.**

### Revised classification count

Combining all verification passes across both sessions: of the 14 listed
`.tex` documents, **1 is an incidental mention** relative to the
canonical `SPEC.md`/`spherepop-kernel` four-primitive event-history
calculus (`quantum-spherepop.tex` — no accompanying formalism of its
own), **5 belong to two non-canonical but historically relevant
Spherepop-branded lineages** (the `polanyi`/`prefigurative`/
`spherepop-calculus`/`spherepop-derivation` quartet, one lineage; the
`ch01`–`ch03` SpherePop-homotopy trio, a second lineage — both real,
internally consistent alternative formalisms, not specifications of the
canonical calculus) — **and 2 are confirmed genuine, substantive
extensions or accurate descriptions of the canonical calculus**
(`calculus/identity_after_collapse.tex`, an independent but name-sharing
primitive calculus; `computation/collapse_retention.tex`, a genuine
extension of `Collapse`). `laboratory/working/spherepop-os.tex` is a
confirmed exact duplicate of this repository's own file (not an
independent document). `alphabet/roadmap/flyxion_briefing.tex` and the
`function_survives_collapse.tex` pair are confirmed genuine, accurate,
non-contradictory references to/reuses of the canonical primitive names,
though at survey/essay level rather than as new formal specifications.
**All 14 `.tex` documents (16 counting exact duplicates) have now been
read and classified in this repository's tracking documents; none remain
unverified.**

**Three-category framework used throughout this document:** (1)
*canonical use* — consistent with `SPEC.md`'s `Pop`/`Refuse`/`Bind`/
`Collapse` option-space calculus or `spherepop-os.tex`'s OS-level
primitives; (2) *non-canonical but historically relevant Spherepop-
branded lineage* — a real, internally consistent, self-contained
formalism that reuses the name but does not implement the canonical
primitives (evidence of genuine branching/semantic reuse, retained as
history, not discarded as noise); (3) *incidental mention* — the name
"Spherepop" or "pop" appears, typically as a one-off metaphor or a named
reference to a design proposal, with no accompanying formalism of its
own to classify as a lineage. A document being non-canonical (category 2
or 3) is not evidence against the canonical calculus's existence — it
only means that document is not itself usable as supporting evidence for
`SPEC.md`'s specific grammar.

## Other substantial (non-`.tex`) essays the source report lists — now verified

The source report separately lists Markdown/`.txt` essays classified as
"substantial essay/source" — most concentrated in
`research-projects/interpretability/` (e.g. `Descripción de
Spherepop.txt` — 259 mentions, 50 formal signals; `Spherepop Essay
LuaLaTeX.txt` — 181 mentions), `antivenom` (`introduction-to-
spherepop.txt` — 177 mentions), and `library` (`physics/spherepop-
calculus-summary.txt` — 77 mentions). `research-projects/epistemology/
Spherepop-OS-extended.txt` (73 mentions, 49 formal signals) is flagged by
the source report as substantive and separate from its own
`compendium/Spherepop-OS-extended.txt` archival duplicate. **All of
these, plus the cross-repository duplicate pairs below, have now been
fetched and read in full:**

10. **`research-projects/interpretability/Descripción de Spherepop.txt` —
    verified: genuine, accurate.** A ChatGPT-assisted drafting session (in
    Spanish and English) explicitly about *this actual repository*
    (`github.com/standardgalactic/spherepop`, cited directly). Correctly
    lists `pop`, `refuse`, `bind`, `collapse`, and `meld` as the event
    vocabulary, states "identity is historical: two things are equal if
    they share the same event history," and describes nested scopes as
    "bubbles." Consistent with the canonical calculus; this is drafting
    material for describing the real project, not an independent spec.

11. **`research-projects/interpretability/Spherepop Essay LuaLaTeX.txt` —
    verified: genuine, accurate, and substantive.** A well-developed essay
    correctly describing history-first identity ("two entities are the
    same if and only if they share the same history of events"),
    append-only event logs as the primary semantic object, and
    `Refuse`-like behavior ("Spherepop refuses to collapse these
    distinctions... Order matters because history matters"). It also
    accurately paraphrases Spherepop OS's actual design goals
    ("deterministic replay, total causal order, and view-cause
    separation"). This is essay-level, not itself a new formal
    specification, but it is consistent with and does not contradict the
    canonical model.

12. **`antivenom/introduction-to-spherepop.txt` ("Groking Sphere Pop," by
    Flixion/GPT, Nov. 2023) — verified: a non-canonical, but historically
    relevant, Spherepop-branded lineage** (the "3D visual programming
    language" lineage), not a false positive in the broad
    which-documents-mention-Spherepop sense. Zero mentions of `refuse`,
    `bind`, `collapse`, or "option space" anywhere in the document.
    Describes "SperePop"/"Sphere Pop" as a marketing pitch for a
    hypothetical **3D visual programming language / educational book**
    where code is visualized as bubbles in 3D space, with only a vague,
    undifferentiated "pop" action (popping releases nested actions). No
    event-history model, no admissibility, no distinct `Refuse`/`Bind`
    primitives — this earlier conception is genuinely a different,
    self-contained branch of the name, not a specification of the
    canonical calculus. **Confirmed byte-identical** (same Git blob SHA
    `8e0c62ab283cd528d2a5293e0758fc836be03fda`) to
    `academizer/stack/spherepop-overview.txt`, matching the source
    report's cross-repository duplicate claim — both are the same
    "3D visual programming language" pitch, not independent essays.

13. **`library/physics/spherepop-calculus-summary.txt` — verified: a
    distinct non-canonical, but historically relevant, Spherepop-branded
    lineage.** "Spherepop Calculus: A Model-Free, Modular Logic of
    Emergent Cognition," explicitly inspired by Monica Anderson's
    Model-Free Methods. Defines its own operation set — `Spawn`, `Merge`,
    `Pop`, `Resurface`, `Diffuse`, `Entangle` — where `Pop` means
    "collapse a sphere to resolve tension, trigger action, or revise
    belief" (conflating Pop and Collapse into one operation). Zero
    mentions of `Refuse` or `Bind`; no option space, no append-only
    history, no Arbiter. This is a third, distinct, cognitive-science-
    flavored "Spherepop" lineage, unrelated to the canonical calculus
    beyond the shared
    name and a vague "popping" metaphor.

14. **`research-projects/epistemology/Spherepop-OS-extended.txt` and
    `research-projects/compendium/Spherepop-OS-extended.txt` — verified:
    byte-identical** (same Git blob SHA
    `a9d24081f482c5b12870391c092766c86f766763`), confirming the source
    report's archival-duplicate claim. Content: "Spherepop OS: A
    Deterministic Semantic Operating System... Formal Specification,
    Architectural Rationale, and Foundations of Pre-Linguistic
    Intelligence" (Flyxion, December 14, 2025), opening with "an
    append-only, causally ordered event substrate from which all semantic
    state is derived" — consistent in framing with this repository's own
    `spherepop-os.tex`, though not confirmed to be textually identical to
    it (not yet diffed against `spherepop-os.tex` line-by-line; treat as
    a *related* but independently-authored document pending that check,
    not as a confirmed duplicate of this repository's own file the way
    `laboratory/working/spherepop-os.tex` was).

## Cross-repository exact duplication (per the source report) — verified

- **`antivenom/spherepop-overview.txt` == `academizer/stack/spherepop-
  overview.txt`: confirmed byte-identical** (SHA `8e0c62ab...`, see item
  12 above) — both the "3D visual programming language" lineage, non-
  canonical relative to `SPEC.md` but a genuine historical branch of the
  name, not incidental noise.
- **`alphabet/sources/The Geometry of Spherepop.txt` ==
  `research-projects/compendium/The Geometry of Spherepop.txt`: confirmed
  byte-identical** (SHA `bbcc5ab23a7767ebc0932e78bd22f482e6b49550`).
  Content: "The Geometry of Spherepop: A Recursive Geometry of Coherence
  in the RSVP Framework... extends the Spherepop Calculus (SPC)"
  (Flyxion Research Group, October 2025). Defines an "SPC DSL" with
  primitives `Sphere`, `Pop`, `Choice` and an appendix "The Spherepop
  Calculus (SPC) Core." Zero mentions of `Refuse`, `Bind`, or "option
  space" — this belongs to the same **non-canonical, but historically
  relevant, "Flyxion/RSVP SPC" branded lineage** already identified for
  `polanyi-spherepop.tex`, `prefigurative-spherepop.tex`,
  `spherepop-calculus.tex`, `spherepop-derivation.tex`, and the
  `ch01`–`ch03` SpherePop-homotopy chapters — not a specification of the
  canonical calculus, but real evidence of a distinct, internally
  consistent semantic branch of the name.
- **`antivenom/diatribe-summary.md` == `kitbash/archive/diatribe-
  summary.md.txt`: confirmed byte-identical** (SHA
  `aa2ef45b0f9ca29695f7d0bbf6f94df8c0dacbc9`). Content: a structural
  critique of Facebook's advertising model, which lists "Spherepop OS: A
  constraint-first interface designed to preserve attentional coherence
  through bounded contexts and low-entropy interaction" as one of two
  proposed alternatives. **This is an incidental mention** — Spherepop OS
  is named as a design proposal with one descriptive sentence, but no
  operator vocabulary, event model, or formalism is given; the essay's
  actual subject is a Facebook critique, not Spherepop.
- **`research-projects/epistemology/briefing-document.md` ==
  `kitbash/archive/briefing-document.md(1).txt`: confirmed
  byte-identical** (SHA `04c4f5162709f1eec32dd708c54174329c8fc17e`).
  "Briefing Document: A Structural Theory of Intelligence, Abstraction,
  and Computation" describes Spherepop OS accurately and consistently
  with the canonical model: "a deterministic semantic operating system
  whose authoritative substrate is an append-only, totally ordered event
  log... Explicit equivalence via MERGE and COLLAPSE," and lists
  "Refusal" among its epistemic design principles. **Verified: a
  genuine, accurate survey-level description** of Spherepop OS (not a new
  specification), analogous to `flyxion_briefing.tex`.
- **`research-projects/epistemology/technical-whitepaper.md` ==
  `kitbash/archive/technical-whitepaper.md(1).txt`: confirmed
  byte-identical** (SHA `37a6ad0c48b1ccbd7fbe51cfa748a4e3aa8db575`).
  "Replayability as a Foundation" devotes its §4 ("Spherepop OS: A
  Realization of Replayable Semantics") to describing the OS kernel's
  actual event vocabulary: "`POP` — introduce a new semantic object;
  `LINK`/`UNLINK` — create or remove relations; `MERGE`/`COLLAPSE` —
  explicit abstraction via equivalence." This matches
  `spherepop-os.tex`'s own kernel-level primitives (as already documented
  in this repository's `COMPLEXITY.md` §13 OS crosswalk). **Verified: a
  genuine, accurate survey-level description**, not a new or divergent
  specification.

## Relationship to this repository's own documents

None of the 14 (or 16-with-duplicates) `.tex` paths above are present in
`standardgalactic/spherepop`. With verification now complete for all 14,
the final classification is:

- **Genuinely distinct from, but name-sharing with, the canonical
  four-primitive calculus** (`SPEC.md`/`IMPLEMENTATIONS.md`'s "Canonical
  basis"): `calculus/identity_after_collapse.tex` (+ its byte-identical
  `working/` copy) — a real `pop`/`refuse`/`bind`/`collapse` "bubble"
  calculus with divergent semantics.
- **Genuine extensions of the canonical `Collapse` operator specifically**:
  `computation/collapse_retention.tex` (+ its byte-identical
  `processing/` copy).
- **Exact duplicate of this repository's own file, not independent**:
  `laboratory/working/spherepop-os.tex`.
- **Genuine, accurate, non-contradictory survey/essay-level references**
  to the canonical primitives (not new specifications):
  `alphabet/roadmap/flyxion_briefing.tex`,
  `alphabet/working/drafts/function_survives_collapse.tex`,
  `alphabet/working/function_survives_collapse.tex`,
  `research-projects/epistemology/briefing-document.md`,
  `research-projects/epistemology/technical-whitepaper.md` (+ their
  byte-identical Kitbash archive copies).
- **Non-canonical, but historically relevant, Spherepop-branded
  lineages** (no option space, no history, no `Pop`/`Refuse`/`Bind`/
  `Collapse` quartet consistent with the canonical model, but each a
  real, internally consistent, self-contained alternative formalism —
  genuine evidence of branching/semantic reuse of the name, not
  discarded as noise):
  - the "Flyxion/RSVP SPC" lineage: `research-projects/polanyi-
    spherepop.tex`, `research-projects/prefigurative-spherepop.tex`,
    `research-projects/spherepop-calculus.tex`,
    `research-projects/spherepop-derivation.tex`,
    `research-projects/RSVP-Monograph/Volume-III-Extensions/chapters/
    ch01-spherepop-homotopy.tex`, `ch02-spherepop-truncation.tex`,
    `ch03-spherepop-computation.tex`, and `alphabet/sources/The Geometry
    of Spherepop.txt` (+ its byte-identical `research-projects/
    compendium/` copy) — single-`pop`/`merge`+`collapse`/`Sphere`-DSL
    variants sharing one author group and RSVP framing;
  - the "3D visual programming language" lineage:
    `antivenom/introduction-to-spherepop.txt` (+ its byte-identical
    `academizer/stack/spherepop-overview.txt` copy);
  - the "Model-Free cognition" lineage:
    `library/physics/spherepop-calculus-summary.txt`.
- **Incidental mentions** (the name appears, with no accompanying
  formalism to classify as a lineage):
  `alphabet/aphantasia/quantum-spherepop.tex` (one-off physics metaphor);
  `antivenom/diatribe-summary.md` (+ its byte-identical Kitbash copy;
  names "Spherepop OS" as one of two proposed design alternatives, no
  operator vocabulary given).

This is a genuinely separate body of work from both:
- the canonical four-primitive lineage documented in `SPEC.md` /
  `IMPLEMENTATIONS.md`'s "Canonical basis," and
- the `personalinks` sphere-path/quotient lineage documented in
  `GRAMMAR.md`.

`calculus`'s "bubble"-based `pop`/`refuse`/`bind`/`collapse` calculus
(verified above) is a **third, independent primitive-based calculus**,
distinct from both existing lineages — worth folding into
`IMPLEMENTATIONS.md`'s specification-lineages section as an additional
family if and when it is read in full. That has not been done yet; this
document now fully verifies the cross-repository `.tex` inventory, the
named Markdown/`.txt` essays, and the 3 duplicate pairs (6 paths) noted
above. No files remain unverified.
