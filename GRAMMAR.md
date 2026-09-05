# GRAMMAR.md — provenance and verification note

**Status: VERIFIED against the now-supplied sources — and provenance
corrected.** The document below (from "## 1. Purpose and authority"
onward) was supplied by the user during this session, attributed to
"Flyxion," as a candidate consolidated grammar for Spherepop's
executable core, causal calculus, and OS kernel event language. It was
initially saved with a DISPUTED/UNVERIFIED status because
`test_spherepop.py`, `spherepop-os.tex`,
`spherepop-commodore-tutorial.pdf`/`.tex`, and `analyze-spherepop.sh`
did not exist anywhere in this repository or environment at that time.
The user then added copies of all of these files at this repository's
root, and each claim below was checked directly against them.

**Correction: the executable-core lineage this document leans on
(§3–7, the sphere-expression/quotient grammar and `test_spherepop.py`)
does not actually belong to `standardgalactic/spherepop`.** It lives in
a separate repository, `standardgalactic/personalinks`, which has its
own `spherepop/` Python package (`grammar.py`, `model.py`,
`semantics.py`, `parser.py`, `views.py`, `validation.py`, `poset.py`,
`predicates.py`, `serialization.py`, `observers.py`, plus 29 numbered
experiment subdirectories under `spherepop/01-grammar/` through
`spherepop/29-multi-timescale-continuation/`) and its own much larger
`tests/` directory (`test_spherepop.py` plus `test_grammar.py`,
`test_parser.py`, `test_semantics_errors.py`, `test_views.py`,
`test_poset.py`, and roughly a dozen more). `personalinks/tests/
test_spherepop.py` was confirmed byte-identical (modulo code
formatting) to the file pasted into this repository. The
`spherepop-os.tex`/`.pdf`, Commodore tutorial, and `analyze-spherepop.sh`
files pasted here also all already exist at `personalinks`' repository
root, alongside its own copy of `Spherepop-OS.tex`. `personalinks` is
described as "conducting preliminary experiments for data analysis";
this Spherepop language work appears to have been developed there
first and only partially, and inconsistently, mirrored into this
dedicated repository (this repo's own `prototypes/python/spherepop/`
package — `core.py`'s `Region`/`Atom`/`Pop`/`Merge`/`CollapseStrategy`
— is a different, unrelated, older implementation that does not
implement the API `test_spherepop.py` expects). Treat this document's
§3–7 claims as verified **against `personalinks`**, not as evidence
about anything currently implemented in this repository.

**Correction, take two: this repository is not implementation-less.**
A prior version of this note, based only on a filename search for
`spherepop` packages, left the impression that this repository has no
executable core at all. That was wrong. `IMPLEMENTATIONS.md` (this
repo's Phase A / Section 5 inventory, each entry backed by reading the
actual source) documents **nine** executable implementation artifacts
or lineages here: three **conforming with the canonical model**
(`spherepop-kernel/` Rust, the normative reference; `experiments/flat/
run_python.py`, the independent Python oracle; `compiler/tools/
fixtures/`, the standalone C fixture kernel — all three agree on all 12
executable flat fixtures); three **experimental extensions**
(`compiler/`'s own interpreter, whose `Bind`/`Collapse` do not yet
conform; `prototypes/sphereforth_gforth.zip`; `reduction-engine/
spherepop.py`, whose `POP`/`REFUSE`/`COLLAPSE` are outcome labels with
different meanings); and three **historical artifacts**
(`prototypes/python/spherepop/`'s `Region` model, `spherepop.hs`,
`spherepop.rkt`/`main.rkt`). The `personalinks` sphere-expression/
quotient lineage this section of `GRAMMAR.md` verifies against is a
**tenth** lineage, and the only one that lives outside this
repository. See `IMPLEMENTATIONS.md` for full detail on all nine
in-repo lineages.

**The real, unresolved issue this raises:** Spherepop now has **two
genuinely different active semantic lineages using the same four
primitive names.** The canonical lineage — `spherepop-kernel/`,
`experiments/flat/run_python.py`, `compiler/tools/fixtures/`, as
specified in `SPEC.md` — defines `Pop`/`Refuse`/`Bind`/`Collapse` over
an integer-object-id option space `Ω` and an append-only event history.
The `personalinks` lineage this document (§3–7) verifies against
defines the same four names over a nested sphere-path/quotient
grammar. Neither is a bug relative to the other; they are two
independently developed calculi that happen to share vocabulary. This
document should not be read as describing the canonical
`standardgalactic/spherepop` semantics — for that, see `SPEC.md` and
`IMPLEMENTATIONS.md`'s "Canonical basis" table — and reconciling (or
formally distinguishing) the two lineages remains open work under
issue #1.

- **§4/§6 executable command grammar — verified against
  `test_spherepop.py`.** `parse_program(["POP 1", "REFUSE o2", "BIND
  prefix:a", "COLLAPSE B=C"])` and every other cited behavior (Pop as
  scope resolution leaving the option space unchanged; Refuse as
  option-space subtraction requiring a nonempty effective target; Bind
  as a predicate filter with an existential reading over Quotient
  members; Collapse as equivalence-quotient formation, not string
  replacement, verified via the `Quotient` type and
  `assertNotIn("B", ...)`/`assertNotIn("C", ...)`; append-only
  `history_is_prefix` monotonicity; `extensional_view` vs. intensional
  `history_view` divergence for `POP 1` vs. `BIND ALL; POP 1`) is
  exercised directly in `test_spherepop.py`. (`test_spherepop_original.py`
  is an earlier, smaller version of the same suite — e.g. it lacks the
  by-label Pop-resolution and post-collapse Bind/Refuse tests — and
  should be read as history, not as an independent second source.)
- **§8–9 kernel event grammar and semantics — verified against
  `spherepop-os.tex`.** The kernel state tuple `σ=(O,U,R,M)`; the
  `POP`/`MERGE`/`LINK`/`UNLINK`/`COLLAPSE`/`SET_META` transition rules;
  the UNLINK reduction as `Collapse(Pop_revoke_r(Bind_f_valid(H_{a,b}
  ⊕ {revoke_r, retain_r})))` opening a **freshly opened option space**
  rather than refusing or erasing the earlier `Pop_{r(a,b)}`
  (Definition "UNLINK Reduction," Proposition "UNLINK Does Not Erase");
  the MERGE reduction and its external identity-designation
  requirement beyond tensor; SET_META's proof that it admits no
  reduction to Pop/Refuse/Bind/Collapse/⊗ and sits outside the causal
  algebra entirely; and the Branch typing judgment `Γ⊢t⇒_e(t_p,t_c):
  Branch` with its shared-prefix coherence axiom and fork/speculation
  identity-policy split — all appear essentially verbatim in
  `spherepop-os.tex`. Note this repo also has an older, much shorter
  `Spherepop-OS.tex` (413 lines vs. `spherepop-os.tex`'s 1291) that
  states the six kernel events and basic transition rules but not the
  UNLINK/MERGE/SET_META reduction proofs, the completeness-scope
  theorem, or the Branch judgment; `spherepop-os.tex` is the fuller,
  authoritative document for those.
- **Commodore tutorial — verified as illustrative-only, not a grammar
  source.** `spherepop-commodore-tutorial-draft.tex` is a from-scratch
  VICE/C64-BASIC tutorial (installing the emulator, `POKE`-based screen
  memory, `GET A$` key polling, a player/projectile/target BASIC game).
  It contains no Spherepop parser, BNF, textual command syntax, typing
  judgment, or desugaring rule; "pop," "collision," and "score" are
  ordinary game-programming vocabulary, not the causal primitives. It
  does not constrain either grammar in this document and should be
  cited only as an application-level illustration, if at all.
- **`analyze-spherepop.sh` — verified as corpus-analysis
  infrastructure, not parser-exercising evidence.** It is a resumable
  pipeline (extraction → canonicalization → summarization → cluster
  synthesis → cross-corpus synthesis → critique → reconstruction) that
  runs an LLM (Ollama/Granite) over this repository's documents. It
  does not parse or execute Spherepop programs and provides no
  independent grammatical evidence.
- **Remaining open conflict:** an inline, shorter draft (also produced
  earlier this session, not saved as a file) modeled the executable
  command grammar as `POP <object-id>` / `REFUSE <object-id> <reason>`
  / `BIND <object-id> <object-id> <tag>` / `COLLAPSE <rule-id>` over
  integer object ids — i.e. the `spherepop-kernel`/`experiments/flat/`
  canonical Pop/Refuse/Bind/Collapse model documented in `SPEC.md`,
  not the sphere-path/quotient-based grammar `test_spherepop.py`
  actually implements. Both are now confirmed real and both are
  genuinely different executable grammars over the same four primitive
  names; this is a real historical/lineage divergence between two
  parts of this repository, not a citation error, and should be
  documented as such (e.g. in the Phase A history/crosswalk work
  tracked by issue #1) rather than resolved by picking one.

This file remains a candidate consolidated document, not a replacement
for `SPEC.md`, which documents the `spherepop-kernel`/C-port/Python-oracle
line's normative semantics for the four primitives. `GRAMMAR.md`
documents a different, now independently verified, executable grammar
(`test_spherepop.py`'s sphere-expression/quotient model) and OS kernel
event layer (`spherepop-os.tex`). Reconciling the two lineages into one
history is follow-up work, not yet done here.

---

# Spherepop Grammar and BNF Specification

**Status:** consolidated working specification, 5 September 2026  
**Author:** Flyxion  
**Scope:** the recent executable core, the typed causal calculus, and the Spherepop OS event language

## 1. Purpose and authority

Spherepop currently has three related but non-identical linguistic layers. The first is the executable core exercised by the Python tests: nested spheres and the commands `POP`, `REFUSE`, `BIND`, and `COLLAPSE`. The second is the abstract causal calculus in which those four names denote typed transformations of a state, together with sequence, tensor composition, and Branch. The third is the operating-system event language, which exposes `POP`, `MERGE`, `LINK`, `UNLINK`, bulk `COLLAPSE`, and `SET_META`, then derives process, namespace, scheduling, module, and capability operations from them.

This document consolidates those layers without pretending that they are already implemented by one parser. Its normative claims use the following labels:

`CORE` means demonstrated by the recent Python test suite. `CALCULUS` means formally specified by the Spherepop OS paper as a typing or semantic rule. `KERNEL` means specified as an authoritative OS event. `DERIVED` means defined by reduction to lower-level terms. `POLICY` means required for a complete implementation but not determined by the four causal primitives. `OPEN` means intentionally unresolved.

The four-primitive completeness claim is narrowly stated: Pop, Refuse, Bind, and Collapse are complete for **causal history transformation**. They are not claimed to express every structural operation, identity decision, interface guarantee, or metadata update.

## 2. Lexical conventions

The executable examples establish symbolic atom names such as `A`, `alpha`, `o2`, and `prefix:a`, decimal path components such as `1`, parentheses for sphere nesting, a colon following an optional sphere label, whitespace separation, and `=` inside a collapse-class declaration. The following lexical BNF is a conservative consolidation of those demonstrated forms.

```bnf
<letter>          ::= "A" | ... | "Z" | "a" | ... | "z"
<digit>           ::= "0" | ... | "9"
<name-start>      ::= <letter> | "_"
<name-rest>       ::= <name-start> | <digit> | "-"
<name>            ::= <name-start> { <name-rest> }
<integer>         ::= <digit> { <digit> }
<ws>              ::= ( " " | "\t" ) { " " | "\t" }
<eol>             ::= "\n" | end-of-input
```

Names are case-sensitive at the semantic level unless an implementation explicitly normalizes them. Keywords are written uppercase in the current command syntax. An implementation may accept lowercase interactively, but canonical serialization uses uppercase keywords.

## 3. Sphere-expression grammar

A sphere expression is an ordered, rooted nesting of atoms and sub-spheres. A label names a sphere scope; it is not itself an atom in that scope.

```bnf
<sphere>          ::= "(" [ <label> ":" <ws> ] <member-list> ")"
<member-list>     ::= <member> { <ws> <member> }
<member>          ::= <atom> | <sphere>
<atom>            ::= <name>
<label>           ::= <name>
```

The demonstrated expressions include:

```text
(A B C)
(A (B C) D)
(outer: A (inner: B C) D)
(outer: (inner: A) (inner: B))
```

The root is addressed by the empty path. A non-root sphere is addressed by a zero-based sequence of child positions. Thus the nested sphere in `(A (B C) D)` has path `(1)`. Labels are resolved globally within the current expression in the demonstrated implementation. A label must resolve to exactly one sphere. Failure to find it and ambiguity between two matching labels are errors; Spherepop does not guess.

## 4. Executable command grammar

The smallest grammar warranted directly by the tests is:

```bnf
<program>         ::= { <blank-line> | <operation-line> }
<blank-line>      ::= [ <ws> ] <eol>
<operation-line>  ::= [ <ws> ] <operation> [ <ws> ] <eol>

<operation>       ::= <pop-op>
                    | <refuse-op>
                    | <bind-op>
                    | <collapse-op>

<pop-op>          ::= "POP" [ <ws> <pop-target> ]
<pop-target>      ::= <path> | <label>
<path>            ::= <integer> { "." <integer> }

<refuse-op>       ::= "REFUSE" <ws> <option-list>
<option-list>     ::= <option-name> { <option-separator> <option-name> }
<option-separator>::= <ws> | [ <ws> ] "," [ <ws> ]
<option-name>     ::= <name>

<bind-op>         ::= "BIND" <ws> <predicate>
<predicate>       ::= "ALL" | "prefix:" <name>

<collapse-op>     ::= "COLLAPSE" <ws> <class-list>
<class-list>      ::= <equivalence-class> { <class-separator> <equivalence-class> }
<class-separator> ::= <ws> | [ <ws> ] "," [ <ws> ]
<equivalence-class> ::= <option-name> "=" <option-name>
                        { "=" <option-name> }
```

Only the concrete forms `POP`, `POP 1`, `REFUSE o2`, `BIND prefix:a`, `BIND ALL`, and `COLLAPSE B=C` are directly demonstrated in the supplied tests. Dotted paths, comma-separated lists, multi-member classes, and multiple classes are the natural canonical completion of the data model, but should be checked against or added to the parser before being treated as implemented syntax.

`POP` with no target is syntactically valid in the current tests and produces a Pop operation whose path is absent. A command must not simultaneously supply a path and a label. Because the line syntax contains only one target slot, simultaneous path-and-label targeting is principally an API-level invalid state.

## 5. Core configuration

The executable semantics operate on a configuration

\[
C = (\sigma,\Omega,H,Q),
\]

where \(\sigma\) is a sphere expression, \(\Omega\) is the current option space, \(H\) is an append-only causal history, and \(Q\) is the collapse or quotient record. An option is either a plain symbolic name or a quotient value:

```bnf
<option>          ::= <option-name> | <quotient>
<quotient>        ::= "{" <option-name> { "," <option-name> } "}"
```

The brace notation is a specification notation, not yet a demonstrated source-language literal. A quotient has no privileged representative field. Implementations may choose a stable display representative—currently the lexicographically first member—but equality is determined solely by the set of members.

Every successful transition appends exactly one indexed event to \(H\). If \(C_0 \to C_1\), then \(H_0\) is a prefix of \(H_1\). Derived views may read configuration state but must not modify authoritative history.

## 6. The four causal operators

### 6.1 Pop

At the executable sphere layer, Pop resolves a named or path-addressed nested scope into its parent. For example:

```text
(A (B C) D)  -- POP 1 -->  (A B C D)
```

This is scope resolution, not deletion. The option space is unchanged, and a Pop event is appended to history.

At the causal-calculus layer, `Pop_x` commits an admissible candidate \(x\) from the current option space into history. At the kernel layer, `POP(o)` creates a new semantic object. These are related uses of commitment, but they are not literally the same data operation. Kernel object creation is reduced to a trivial Bind over `{exists_o, absent_o}` followed by `Pop_exists`.

### 6.2 Refuse

For requested names \(R\), executable Refuse subtracts every currently represented option whose member set intersects \(R\):

\[
\Omega' = \{q\in\Omega \mid \operatorname{members}(q)\cap R=\varnothing\}.
\]

The effective target must be nonempty. `REFUSE` without operands is a parse error. If every requested name is absent from the option space, evaluation fails because no refusal occurred. If a requested name is a member of a quotient, the entire quotient is removed. History records the names explicitly requested, not every collateral member of a removed quotient.

Refuse acts only on the **current option space**. It cannot delete or rewrite an earlier Pop already in history. Revocation is therefore expressed by opening a fresh option space concerning continued standing and committing a revocation event, not by refusing the historic commitment.

### 6.3 Bind

Bind filters the current option space by a predicate:

\[
\Omega' = \{q\in\Omega \mid \widehat f(q)\}.
\]

For a plain option, \(\widehat f\) is the ordinary predicate. For a quotient, the current executable interpretation is existential:

\[
\widehat f([x]) \iff \exists a\in[x]\; f(a).
\]

Thus `BIND prefix:a` admits the quotient `{alpha,gamma}` because at least one member matches. `ALL` is the identity predicate. The existential quotient rule is provisional application semantics rather than a consequence of the four-operator grammar.

### 6.4 Collapse

Executable Collapse replaces declared equivalent options with an extensional quotient. `COLLAPSE B=C` transforms an option space `{A,B,C}` into `{A,{B,C}}`. Rendering may display `(A B B)` by using `B` as a stable representative, but downstream semantics receive the quotient itself and cannot distinguish it by comparing it directly with the string `B`.

Collapse is therefore quotient formation or observable projection, not destruction. At the kernel level, bulk `COLLAPSE(S,o_r)` additionally assigns a canonical representative in the union–find map. That identity-designation step is external policy, making kernel Collapse a hybrid rather than a direct occurrence of causal Collapse.

## 7. Abstract term grammar

The typed causal calculus can be written as the following abstract grammar. This is a term grammar, not a promise that the current line parser accepts the notation.

```bnf
<state-term>      ::= <state-name>
                    | "Bind" "[" <predicate-id> "]" "(" <state-term> ")"
                    | "Pop" "[" <candidate-id> "]" "(" <state-term> ")"
                    | "Refuse" "[" <candidate-set> "]" "(" <state-term> ")"
                    | "Collapse" "(" <state-term> ")"
                    | <state-term> ";" <state-term>
                    | <state-term> "⊗" <state-term>
                    | <projection>

<branch-term>     ::= "Branch" "[" <event-id> "]"
                      "(" <state-term> ";" <state-term> "," <state-term> ")"
<projection>      ::= "πp" "(" <branch-term> ")"
                    | "πc" "(" <branch-term> ")"
```

Its principal judgments are:

\[
\Gamma\vdash t:\mathsf{State},\qquad
\Gamma\vdash t\Longrightarrow_e(t_p,t_c):\mathsf{Branch}.
\]

The operator typing rules are schematically:

\[
\frac{\Gamma\vdash t:\mathsf{State}}{\Gamma\vdash\operatorname{Bind}_f(t):\mathsf{State}},\quad
\frac{\Gamma\vdash t:\mathsf{State}\quad x\in\operatorname{Opt}(t)}{\Gamma\vdash\operatorname{Pop}_x(t):\mathsf{State}},
\]

\[
\frac{\Gamma\vdash t:\mathsf{State}\quad\varnothing\ne R\subseteq\operatorname{Opt}(t)}{\Gamma\vdash\operatorname{Refuse}_R(t):\mathsf{State}},\quad
\frac{\Gamma\vdash t:\mathsf{State}}{\Gamma\vdash\operatorname{Collapse}(t):\mathsf{Obs}}.
\]

Sequence and tensor are structural rules:

\[
\frac{\Gamma\vdash t_1:\mathsf{State}\quad\Gamma\vdash t_2:\mathsf{State}}
{\Gamma\vdash t_1\otimes t_2:\mathsf{State}}.
\]

Tensor combines independently constructed histories. It makes no selection, refusal, narrowing, projection, or identity judgment, so it is not a fifth causal primitive.

Branch introduces two continuations of one history:

\[
\frac{\Gamma\vdash t:\mathsf{State}}
{\Gamma\vdash t\Longrightarrow_e(t_p,t_c):\mathsf{Branch}}.
\]

Both projections must share the exact prefix that existed at the branch event:

\[
H_{t_p}|_{|H_t|}=H_{t_c}|_{|H_t|}=H_t.
\]

Branch is not an unconstrained product and is not the inverse of tensor. Recombining the projections preserves both post-branch continuations. Identity behavior is supplied at the use site: `fork` preserves the parent's identity and mints a child identity, whereas speculative execution gives both projections the identity of the same object.

## 8. Kernel event grammar

The authoritative kernel event family is:

```bnf
<kernel-event>    ::= "POP" "(" <object-id> ")"
                    | "MERGE" "(" <object-id> "," <object-id> ")"
                    | "LINK" "(" <object-id> "," <object-id> "," <relation-type> ")"
                    | "UNLINK" "(" <object-id> "," <object-id> "," <relation-type> ")"
                    | "COLLAPSE" "(" <object-set> "," <object-id> ")"
                    | "SET_META" "(" <object-id> "," <meta-key> "," <meta-value> ")"

<object-set>      ::= "{" <object-id> { "," <object-id> } "}"
<object-id>       ::= <name>
<relation-type>   ::= <name>
<meta-key>        ::= <name>
<meta-value>      ::= implementation-defined canonical value
```

The kernel state is \(\sigma=(O,U,R,M)\), where \(O\) is the finite object set, \(U\) is the union–find representative map, \(R\) is a multiset of typed directed relations, and \(M\) is a partial metadata map. The sole arbiter assigns a strictly increasing event identifier to every accepted event. State is derived only by deterministic replay of the authoritative append-only log.

Kernel `POP` adds a fresh object. `MERGE` induces equivalence and designates one representative. `LINK` creates a relation between current representatives. `UNLINK` makes that relation inactive without erasing the earlier LINK from history. Bulk `COLLAPSE` induces equivalence across a finite region and designates a representative. `SET_META` changes only the annotation map.

## 9. Reductions and classification

`LINK(a,b,r)` has the causal shape

\[
\operatorname{Collapse}(\operatorname{Refuse}_{r'}(
\operatorname{Pop}_{r(a,b)}(
\operatorname{Bind}_{C_r}(H_a\otimes H_b)))).
\]

`UNLINK(a,b,r)` opens a fresh `{revoke_r, retain_r}` option space over the joint relation history, filters it for validity, and Pops `revoke_r`. It never targets the historic `Pop_r(a,b)`.

`MERGE(o_1,o_2)` filters the tensor-composed histories for identity-compatible continuations and refuses the distinct alternative, but additionally requires a representative-selection policy. Bulk kernel Collapse requires the same kind of policy.

The resulting taxonomy is authoritative for this consolidation:

| Tier | Members |
|---|---|
| Causal operators | Pop, Refuse, Bind, Collapse |
| Structural rules | sequence, tensor, Branch |
| Direct kernel primitive | `POP` |
| Structurally derived kernel events | `LINK`, `UNLINK` |
| Hybrid events | `MERGE`, bulk `COLLAPSE(S,o_r)` |
| Non-causal annotation | `SET_META` |
| Derived kernel functions | `fork`, `exec`, `exit`, `wait`, `schedule`, `write`, `read`, `rename`, `delete`, `mount`, `grant`, `revoke` |
| Interface mechanisms | syscall proposals, atomic composition, driver admission |

`SET_META` is deliberately outside the causal completeness claim. It modifies no option space and appends nothing to an object's causal history. Treating metadata as a fictional single-option causal choice would conceal the boundary rather than formalize it.

## 10. Supersession, revocation, and observation

The primitives preserve history but do not by themselves determine which of several conflicting commitments should be reported as current. Spherepop OS presently adopts **most-recent-event precedence** for relation slots: the latest committed event concerning `r(a,b)` or `revoke_r` determines whether the relation is active.

This is a collapse policy, not a theorem of Pop, Refuse, Bind, and Collapse. Likewise, whether content commits replace, append to, or patch earlier content is determined by a version-pinned semantic library for the object type.

Two histories may therefore have the same extensional view while remaining intensionally distinct. For example, `POP 1` and `BIND ALL; POP 1` may render the same sphere and option space, but their histories differ. Spherepop preserves that difference.

## 11. Derived OS surface

The following abstract interface grammar records the operations named in the current OS specification. It is descriptive and should not be confused with the implemented four-command parser.

```bnf
<derived-call>    ::= "fork" "(" <process-id> ")"
                    | "exec" "(" <process-id> "," <object-id> ")"
                    | "exit" "(" <process-id> "," <value> ")"
                    | "wait" "(" <process-id> "," <process-id> ")"
                    | "schedule" "(" <candidate-set> ")"
                    | "write" "(" <path-value> "," <value> ")"
                    | "read" "(" <path-value> ")"
                    | "rename" "(" <path-value> "," <path-value> ")"
                    | "delete" "(" <path-value> ")"
                    | "mount" "(" <path-value> "," <object-id> "," <version-id> ")"
                    | "grant" "(" <identity-id> "," <capability-id> ")"
                    | "revoke" "(" <identity-id> "," <capability-id> ")"
```

These operations submit proposals through a syscall or driver boundary. Admission is modeled as Bind against permission and validity predicates; acceptance Pops the proposed operation, while rejection Refuses the admission candidate. `delete(p)` is path-named UNLINK rather than object erasure. `grant` is LINK in the `grants` relation and `revoke` is corresponding UNLINK.

Atomic composition, written \(\langle\!\langle t_1;t_2\rangle\!\rangle\), admits several internal actions under one EID so that no Collapse can observe an intermediate state. This is an Arbiter/interface capability, not something derived from the causal or structural grammar.

## 12. Static and dynamic validity

A conforming implementation should distinguish parse errors from evaluation errors. Missing required syntax, malformed nesting, or an empty textual `REFUSE` is a parse error. A well-formed target that does not exist, an ambiguous label, an effective refusal target that is empty, or a Pop request specifying mutually exclusive targeting modes is an evaluation error.

Successful transitions satisfy append-only history, deterministic replay under a fixed semantic-library version, non-mutating derived views, and quotient extensionality. Replay determinism is conditional on every event being interpreted by the semantic-library version attributed to it. Loading a new module must not reinterpret old events under new evaluator semantics.

Namespace isolation additionally requires coherent renderers. Different views may mask unauthorized fact domains and use different presentation maps, but they must share a canonical version-pinned semantic evaluator for facts visible to both. Determinism of each view separately does not prevent two views from deterministically contradicting one another.

## 13. Canonical serialization recommendations

Canonical source should use uppercase operation keywords, one command per line, a single ASCII space between a keyword and its operand, zero-based dotted paths, labels immediately followed by `:`, comma-separated refusal operands and collapse classes, and `=` between members of an equivalence class. Canonical kernel serialization should make object identifiers, relation types, metadata value types, EIDs, and semantic-library versions explicit rather than relying on presentation labels.

These conventions close representational gaps but do not alter semantics. Before they are declared language version 1.0, the parser should receive conformance tests for dotted paths, commas, multi-member quotient classes, multiple collapse classes, comments, blank lines, duplicate names, escaping, Unicode identifiers, and round-trip canonicalization.

## 14. Open points requiring a language decision

The recent materials intentionally leave several questions unresolved. The grammar does not yet state whether an empty sphere is legal; whether atoms may be quoted strings; whether labels have lexical scope or global-expression scope; whether duplicate atom names denote repeated occurrences or one identity; whether comments are accepted; or whether identifiers may contain Unicode, slashes, dots, and colons.

The semantic layer also leaves open the general identity-designation rule for MERGE and bulk Collapse; the scheduling priority function among several admissible candidates; contention resolution among simultaneously valid LINK or mount proposals; per-type content supersession semantics; and reclamation of retained but permanently superseded history.

None of these gaps invalidates the core. They locate the boundary between a working calculus and a fully versioned programming language. The most important next consolidation step is to choose one textual syntax for the abstract calculus and kernel events, implement it beside the existing four-command parser, and make this document executable as a conformance suite.

## 15. Compact reference

The shortest faithful summary is:

```text
Sphere expression : ordered nested scopes, optionally labeled
Core configuration: sphere + option space + append-only history + quotients
Causal basis      : Pop / Refuse / Bind / Collapse
Structural basis  : sequence / tensor / Branch
Kernel events     : POP / LINK / UNLINK / MERGE / COLLAPSE / SET_META
Identity policy   : external for MERGE, bulk COLLAPSE, and Branch use sites
Current relation  : latest committed link-or-revoke event wins
Authority         : one arbiter, one totally ordered append-only log
Views             : pure, version-pinned, coherently evaluated, presentation-variable
```

This is the consolidated Spherepop grammar as supported by the supplied recent materials. Where it extends beyond syntax directly exercised by the tests, it says so explicitly; where the calculus requires policy rather than grammar, it keeps that policy visible instead of smuggling it into the four primitives.
