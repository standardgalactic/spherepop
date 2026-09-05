# COMPLEXITY.md — a crosswalk between Spherepop's notions of "complexity"

**Status: proposed formalization, not yet part of the canonical spec.**
This document exists because prior writing about Spherepop (in this
session and elsewhere in the repository) sometimes moves too quickly
between several genuinely different quantities — event count, structural
depth, constraint, entropy reduction, and assembly — as if they were
interchangeable. They are related, but they are not the same thing. This
document is deliberately **not** a philosophical essay (that role belongs
to prose papers such as `standardgalactic/calculus`'s
`identity_after_collapse.tex`, see §11). It is meant as the precise
crosswalk that prevents the repository's different complexity claims from
drifting back together: every symbol below is given an explicit domain,
every claim is either proved from the definitions or flagged as
unverified, and every scalarization is required to name the policy under
which it was produced. Nothing here has been adopted into `SPEC.md`
itself, and no kernel code implements it — the four primitives'
admissibility and replay semantics are entirely untouched. §14's derived,
non-authoritative analysis functions ($L$, $D_a$, $W_a$, $A_{\mathrm{tree}}$,
$A_{\mathrm{DAG}}$, $A_{\min}^c$) **are** now implemented and fixture-tested
in `experiments/flat/profile.py` / `run_profile.py`, isolated from the
main conformance suite; this remains a conceptual/notational proposal for
Phase B/C work under issue #1 with respect to the kernel and `SPEC.md`
themselves.

**Correction to an earlier version of this document:** a previous draft
stated that the `extensional_view`/`history_view` divergence discussed in
§4 was "exercised in `test_spherepop.py`-style fixtures." That was wrong.
`test_spherepop.py` belongs to the `personalinks` nested-sphere/quotient
lineage (see `GRAMMAR.md`), not the canonical `SPEC.md` kernel lineage this
document is built on. The canonical evidence for the canonical lineage is
the flat conformance fixtures and the three conforming implementations:

```text
spherepop-kernel/
experiments/flat/run_python.py
compiler/tools/fixtures/
```

Checked directly in this session: **there is no flat fixture yet that
exercises an intensional/extensional (`history_view` vs.
observation-under-`Collapse`) divergence.** The closest existing fixture
pair is `experiments/flat/fixtures/05a_same_snapshot_different_history_run_a.json`
/ `05b_same_snapshot_different_history_run_b.json` ("same snapshot,
different history"). The precise relationship is:

> Fixtures 05a and 05b demonstrate that distinct histories may produce the
> same tested snapshot. They therefore support the *general* distinction
> between historical and extensional identity used throughout this
> document, but they do not implement or test the named `extensional_view`
> and `history_view` interface, which belongs to the separate
> `personalinks` lineage (see `GRAMMAR.md`) and has no counterpart in the
> canonical flat fixtures.

§4 and §14 below identify the still-missing fixture (one that exercises a
named `Collapse`-relative view distinction, not merely snapshot equality)
as a concrete gap rather than implying it is already covered.

## 1. Formal setting

Before defining any complexity measure, fix the objects the rest of this
document quantifies over.

A **history** is a finite sequence of admitted events drawn from the
canonical four-primitive alphabet:

$$
H = e_1 e_2 \cdots e_n \in E^{*}, \qquad
E = E_{\mathrm{Pop}} \sqcup E_{\mathrm{Refuse}} \sqcup E_{\mathrm{Bind}} \sqcup E_{\mathrm{Collapse}}.
$$

**Replay** produces an authoritative state:

$$
\operatorname{replay}(H) = S_H.
$$

A **certified collapse rule** $c$ produces an observation:

$$
C_c(H) \in O_c.
$$

From $H$ (or equivalently from $S_H$, if replay is taken as the source of
truth — see the caveat below) four derived objects are read off:

- the **residual option space** $\Omega_H$ — the options not yet
  committed by a `Pop` and not excluded by admissibility failure;
- the **refusal record** $F_H$ — the sequence or multiset of `Refuse`
  events and their recorded reasons;
- the **binding relation** $B_H \subseteq \mathrm{Obj} \times \mathrm{Obj}$
  — the pairs coupled by `Bind` events, *without* asserting identity
  between them (this is the canonical meaning of `Bind`; see §7 for why
  it must not be silently treated as a dependency/assembly relation);
- the **observation record** $Q_H$ — the sequence of `Collapse` events
  applied and the rules $c$ they invoked.

**Open question, not resolved here:** whether $\Omega_H$, $F_H$, $B_H$,
$Q_H$ should be understood as *reconstructed from* the append-only event
log $H$ on demand, or as *materialized fields of* the implementation's
authoritative state $S_H$, is an implementation choice this document does
not settle. `spherepop-kernel`'s `History`/`Arbiter` types
(`src/history.rs`, `src/arbiter.rs`) materialize an analogous structure
directly; whether that is the only legitimate choice, or merely the
current implementation's choice, is left open. This distinction matters
for §13's comparison with the OS kernel-state tuple: `spherepop-os.tex`'s
per-object State term $t=(I_t,\mathrm{Opt}_t,H_t)$ (found and cited in
§13) gives one concrete instance where $\Omega_H$ *is* materialized
(as $\mathrm{Opt}_t$) and $F_H$ is reconstructible on demand (by filtering
$H_t$ for `rej:` tags), while $B_H$ is **neither** — `Bind`'s filter
predicate leaves no trace in $t$ at all, materialized or otherwise.

## 2. Four quantities that must not be conflated

$$
\text{event length} \;\neq\; \text{weighted historical cost} \;\neq\;
\text{algorithmic complexity} \;\neq\; \text{observed complexity}.
$$

**Event length** is raw count:

$$
L(H) = |H|, \qquad L(H \cdot e) = L(H) + 1.
$$

**Weighted historical cost** requires an explicit, positive per-event
weight function $w : E \to \mathbb{R}_{>0}$:

$$
L_w(H) = \sum_{e \in H} w(e).
$$

Append monotonicity then follows *by construction*, not by assumption:

$$
L_w(H \cdot e) = L_w(H) + w(e) > L_w(H).
$$

**Algorithmic complexity** is a genuinely different quantity and must be
named separately:

$$
K_{\mathrm{alg}}(\operatorname{encode}(H)).
$$

A previous version of this document used the symbol $K_{\mathrm{hist}}$
for an "always increases under extension" ledger. That was a naming
error: $K$ conventionally denotes Kolmogorov complexity, and Kolmogorov
complexity is *not* monotone under arbitrary extension in the strong
sense that was apparently intended — a long history containing many
repeated events can have a short description
($K_{\mathrm{alg}}(\operatorname{encode}(H\cdot e))$ need not exceed
$K_{\mathrm{alg}}(\operatorname{encode}(H))$, e.g. if $e$ extends a
periodic pattern). The intended "always increases" claim is true only of
$L$ and $L_w$ (both additive ledgers by construction), never claimed of
$K_{\mathrm{alg}}$. Every later use of a "historical growth" claim in this
document uses $L$ or $L_w$, never $K_{\mathrm{alg}}$.

**Observed complexity** is whatever a chosen measurement $K$ (not
necessarily Kolmogorov complexity — any fixed measurement convention)
assigns to a `Collapse`-produced observation, $K(C_c(H))$. It is a
property of the *observation*, not of the history, and different
observations of the same history under different rules $c$ can disagree
arbitrarily (§8).

## 3. Assembly in the historical sense

Spherepop is a calculus of assembly only in the historical sense:

> Spherepop is a calculus of assembly only in the historical sense. An
> object is not identified merely by its final configuration but by the
> admissible event history through which it was constructed. Assembly
> therefore supplies identity, not merely cost. Two extensionally
> identical structures may have different Spherepop identities when
> their histories differ, while a specified Collapse may deliberately
> treat them as equivalent for a chosen observation.

This connects the calculus directly to assembly theory while marking a
real difference from it: a conventional assembly index asks for the
shortest construction pathway permitted by a repertoire of operations and
reusable components, whereas Spherepop ordinarily preserves the *actual*
path taken, not the shortest one available.

## 4. Minimal assembly is Collapse-relative, not absolute

An earlier version of this document defined minimal assembly as

$$
A_{\min}(x) = \min_{H:\,\operatorname{out}(H)=x} |H|,
$$

silently assuming that an evaluation map $\operatorname{out}$ determines
an unambiguous object $x$. That is precisely what Spherepop questions:
whether two histories "produce the same object" is only well-defined
relative to a chosen observation. The corrected definition parameterizes
minimal assembly by an explicit collapse rule $c$:

$$
A_{\min}^{c}(x) = \min\bigl\{ L_w(H) : C_c(H) = x \bigr\}.
$$

**Proposition (Collapse-relative assembly complexity).** Say $c_1$ is
*finer than* $c_2$, written $c_1 \sqsubseteq c_2$, when every distinction
$c_1$ preserves is also preserved by $c_2$ — equivalently (see §6's
$D_c$), $D_{c_1}(H) \subseteq D_{c_2}(H)$ for every $H$. Then for any $x$
meaningful in both codomains $O_{c_1}$ and $O_{c_2}$:

$$
c_1 \sqsubseteq c_2 \quad\Longrightarrow\quad A_{\min}^{c_1}(x) \;\ge\; A_{\min}^{c_2}(x).
$$

*Reasoning:* $c_1$ imposes a stricter equivalence condition than $c_2$, so
the set of histories $H$ satisfying $C_{c_1}(H) = x$ is no larger than the
set satisfying $C_{c_2}(H) = x$ — a finer observer restricts, never
widens, the admissible construction histories counted toward the minimum.
Minimizing $L_w$ over a subset can only produce a value greater than or
equal to minimizing over a superset. $\blacksquare$

This is one of this document's two central results (see §8 for the
other): **apparent minimal assembly complexity decreases as the observer
is permitted to forget more distinctions.** There is no observer-
independent minimal assembly index unless the criterion of equivalence
has already been fixed.

## 5. Three orders and preorders, not one order of complexity

An earlier version of this document called all three of the following
relations "partial orders." That overstated what follows from the
definitions alone; each is analyzed separately below.

**Causal order** — a genuine partial order, *provided* it is defined over
event occurrences in an acyclic dependency structure (which the canonical
model guarantees: history is append-only and an event can only depend on
events already admitted, so no cycle can form):

$$
e_i \preceq_H e_j \quad\text{when } e_i \text{ must already have occurred
for } e_j \text{ to be admissible}.
$$

**Assembly relation** — initially only a **preorder**, not a partial
order:

$$
x \preceq_A y \quad\text{when some history constructing } x \text{ can be
extended admissibly into one constructing } y.
$$

Reflexivity and transitivity are immediate from admissible extension
being reflexive and transitive. Antisymmetry does **not** follow: two
differently presented structures may each be constructible from the
other under different admissible continuations (e.g. via different
`Collapse` policies), so $x \preceq_A y$ and $y \preceq_A x$ can both hold
without $x = y$. A genuine partial order requires quotienting first (see
below).

**Observational relation** — also initially only a **preorder**. Define
the *distinction set* exposed by rule $c$ over history $H$:

$$
D_c(H) = \bigl\{ (a,b) : C_c(H) \text{ distinguishes } a \text{ from } b \bigr\}.
$$

Then:

$$
H_1 \preceq_c H_2 \iff D_c(H_1) \subseteq D_c(H_2).
$$

This is a preorder (reflexive and transitive by set-inclusion) unless
observational equality has already been quotiented out. On equivalence
classes under

$$
H_1 \sim_c H_2 \iff D_c(H_1) = D_c(H_2),
$$

the induced relation $\preceq_c$ **is** a partial order, since inclusion
is antisymmetric between distinct sets.

**The general pattern:** `Collapse` is exactly what turns a preorder into
a partial order, by supplying the equivalence $\sim_c$ that quotients out
the cases where two histories are mutually related but not identical.
This is a nontrivial consequence connecting observational resolution to
apparent complexity, and is why this section's title says "orders *and
preorders*," not "three orders": only the causal relation is a genuine
order prior to any `Collapse`-induced quotient.

These relations need not agree with one another: later does not always
mean more complex; more assembled does not always mean more
distinguishable; and `Collapse` can shrink $D_c$ without reversing or
erasing the construction history (§8).

## 6. Historical growth vs. observational simplification

> Spherepop complexity is monotone at the level of authoritative history
> but need not be monotone at the level of observed structure. Every
> admitted event increases historical information, even when its result
> is a simpler quotient, a smaller option space, or a more compressed
> view.

Using the corrected ledger from §2 (not $K_{\mathrm{hist}}$):

$$
L_w(H \cdot e) > L_w(H) \quad\text{for every admissible } e,
$$

while for an observation $C_c$ under collapse rule $c$, using a fixed
measurement convention $K$ over observations (§2):

$$
K\bigl(C_c(H \cdot e)\bigr) < K\bigl(C_c(H)\bigr)
$$

may hold. `Collapse` therefore does not make the past less complex — it
makes a selected *observation* less discriminating. See §9 for a concrete
worked witness of this claim.

## 7. `Bind`, dependency, and the assembly-interpretation gap

Canonical `Bind` couples two elements without identifying them — it does
**not** assert that one was assembled from the other. Reading a
dependency/assembly graph out of `Bind` events therefore requires an
additional, explicit interpretation. This document distinguishes:

$$
G_H^{\mathrm{bind}} = (V_H, B_H)
$$

— the graph whose edges are literally the canonical `Bind` relation
$B_H$ from §1, asserting only coupling, not construction — from

$$
G_H^{\mathrm{assembly}} = \mathcal{A}_a\bigl(G_H^{\mathrm{bind}}, H\bigr)
$$

— a *derived* view produced by an explicit interpretation rule $a$ that
selects which bindings (and possibly which `Pop` events) count as
construction dependencies. Different choices of $a$ can read very
different assembly structures out of the same $G_H^{\mathrm{bind}}$: a
social binding, a logical constraint, a causal dependency, a spatial
adjacency, and a genuine assembly dependency are all expressible as
`Bind` events, and nothing in the canonical primitive itself
distinguishes them. All assembly-graph quantities below ($D_a$, $W_a$,
$N_a$, $B_a$ in §8; the dependency-depth claims in §3 of the retracted
draft) are therefore properties of $G_H^{\mathrm{assembly}}$ under a
*named* $a$, never properties of $G_H^{\mathrm{bind}}$ alone.

Concrete evidence for exactly this gap now exists outside this document:
`spherepop-os.tex`'s per-object causal State term (§13, Definition "State
Term, Extended") defines $\mathrm{Bind}_f(t) = (I_t, \{o \in
\mathrm{Opt}_t : f(o)\}, H_t)$ — note $H_t$ on the right-hand side is
**unchanged**. Unlike `Pop` and `Refuse`, which each append a tagged
entry to the history component, `Bind`'s filter predicate $f$ leaves no
trace once applied. This is an independent, source-verified confirmation
that $B_H$/$G_H^{\mathrm{bind}}$ cannot in general be recovered from a
replayed state or a materialized history component alone — recovering it
requires retaining the full external operation sequence, exactly the
caution this section raises in the abstract.

## 8. The complexity profile, split into invariant and policy-relative parts

$$
\mathbf{A}_{a,c}(H) = \bigl(
L(H),\; L_w(H),\; D_a(H),\; W_a(H),\; N_a(H),\; B_a(H),\; F(H),\;
|\Omega_H|,\; U_a(H),\; Q_c(H)
\bigr).
$$

| Symbol | Meaning | Depends on |
|---|---|---|
| $L(H)$ | raw event count | — (invariant) |
| $L_w(H)$ | weighted historical cost | a weight function $w$ |
| $D_a(H)$ | assembly depth (longest dependency path in $G_H^{\mathrm{assembly}}$) | assembly interpretation $a$ |
| $W_a(H)$ | concurrency width (maximum antichain width in $G_H^{\mathrm{assembly}}$) | assembly interpretation $a$ |
| $N_a(H)$ | number of distinct constructed components (vertices of $G_H^{\mathrm{assembly}}$) | assembly interpretation $a$ |
| $B_a(H)$ | dependency count (edges of $G_H^{\mathrm{assembly}}$) | assembly interpretation $a$ |
| $F(H)$ | refusal burden, $|F_H|$ or a weighted variant | — (invariant given a weight on refusals) |
| $|\Omega_H|$ | residual optionality | — (invariant, per §1's caveat about reconstruction vs. materialization) |
| $U_a(H)$ | reuse gain (§9) | assembly interpretation $a$ |
| $Q_c(H)$ | observational resolution, e.g. $|D_c(H)|$ | collapse rule $c$ |

The subscripts are not decoration: $D_a, W_a, N_a, B_a, U_a$ depend on a
*named* assembly interpretation, and $Q_c$ depends on a *named* collapse
rule. $L$, $L_w$, $F$, $|\Omega_H|$ are the only entries that do not
require declaring $a$ or $c$ — call these the profile's **invariant
core**.

A scalarization must declare **two** separate policies, not one:

$$
A_{\phi,a,c}(H) = \phi\bigl(\mathbf{A}_{a,c}(H)\bigr).
$$

$c$ (the collapse rule) determines *observational equivalence* — which
distinctions the profile is even allowed to see. $\phi$ (the weighting
function) determines *how the resulting heterogeneous dimensions are
valued and combined* — event count against dependency depth against
reuse against residual optionality. These are related but distinct
policies: `Collapse` does not, by itself, contain every value judgment
a scalar complexity score requires.

### Primitive-specific effect signatures

For an admitted event $e$, write $\Delta_e \mathbf{A} = \mathbf{A}(H\cdot
e) - \mathbf{A}(H)$ (componentwise, where subtraction is meaningful).
None of the four primitives moves every coordinate in one fixed
direction; each affects a specific, limited subset:

$$
\Delta_{\mathrm{Pop}} L > 0, \qquad \Delta_{\mathrm{Pop}} |\Omega| \le 0.
$$

$$
\Delta_{\mathrm{Refuse}} L > 0, \qquad \Delta_{\mathrm{Refuse}} F \ge 0,
\qquad \Delta_{\mathrm{Refuse}} |\Omega| = 0
$$

— the last equality holds under the current canonical interpretation, in
which refusal records inadmissibility *without deleting the option from
the underlying possibility space* (`SPEC.md`; consistent with
`Spherepop_Specifications.tex`'s conservation-of-possibility framing, §12
below).

$$
\Delta_{\mathrm{Bind}} L > 0, \qquad \Delta_{\mathrm{Bind}} B_{\mathrm{bind}} \ge 0,
$$

where $B_{\mathrm{bind}} = |E(G_H^{\mathrm{bind}})|$; assembly depth
$D_a$ changes only if the chosen interpretation $a$ treats that
particular binding as a dependency — a `Bind` event is not guaranteed to
move any assembly-graph coordinate at all.

$$
\Delta_{\mathrm{Collapse}} L > 0,
$$

but $K(C_c(H\cdot e))$ relative to $K(C_c(H))$ is **unconstrained** by the
primitive alone: it may be smaller, equal, or (for a rule that reveals
more structure than it hides) larger. `Collapse` must not be declared
universally complexity-reducing; it is *distinction-reducing relative to
a particular observational comparison* $D_c$, and $D_c$ can shrink while
$K$ under some unrelated measurement convention does not.

## 9. Reuse: tree unfolding vs. dependency DAG

An earlier version of this document proposed a subtractive reuse-credit
formula,

$$
A_{\mathrm{reuse}}(H) = \sum_{v \in V(G_H)} w(v) - \sum_{u \in R(H)}
(m_H(u)-1)\,\rho(u),
$$

which has two defects: it can go negative, and it does not specify
whether the first sum already counts a shared component once or once per
use. The corrected treatment compares two different graphs built from the
same history.

Let $T_H$ be the **unfolded construction tree** — obtained from
$G_H^{\mathrm{assembly}}$ (§7) by duplicating every reused component at
every point of invocation, so that no vertex is shared:

$$
A_{\mathrm{tree}}(H) = \sum_{v \in V(T_H)} w(v).
$$

Let $A_{\mathrm{DAG}}(H)$ price the dependency DAG *as actually
constructed*, with reuse represented by edges rather than duplicated
vertices:

$$
A_{\mathrm{DAG}}(H) = \sum_{v \in V(G_H^{\mathrm{assembly}})} w(v) +
\sum_{e \in E(G_H^{\mathrm{assembly}})} \lambda(e),
$$

where $\lambda(e)$ is the cost of binding to (reusing) an existing
component, usually $\lambda(e) < w(v)$ for the component $v$ it reuses.
The **reuse gain** is then a plain, non-negative-by-construction
difference:

$$
U_a(H) = G_{\mathrm{reuse}}(H) = A_{\mathrm{tree}}(H) - A_{\mathrm{DAG}}(H)
\;\ge\; 0
$$

whenever $\lambda(e) \le w(v)$ for every reused $v$ — reuse is not
"negative assembly," it is the difference between reconstructing every
occurrence independently and constructing shared components once with
additional dependency links.

## 10. Three worked counterexamples

**Counterexample 1 — equal length, different depth and width.** Two
histories, each with four `Pop`/`Bind` events ($L(H_1) = L(H_2) = 4$).
$H_1$ is sequential ($a \to b \to c \to d$, each depending on the last):
$D_a(H_1) = 4$, $W_a(H_1) = 1$. $H_2$ constructs four independent
components with no bindings between them: $D_a(H_2) = 1$, $W_a(H_2) = 4$.
Equal event count, opposite depth/width profile — event count alone
cannot distinguish "long sequential chain" from "wide independent batch."

**Counterexample 2 — reuse vs. reconstruction under a coarse Collapse.**
$H_1$ constructs component $u$ once (from $a, b$) and binds it into three
downstream assemblies. $H_2$ independently reconstructs an $a$-$b$
assembly three times, once per downstream use. A sufficiently coarse
collapse rule $c$ can make the two histories observationally identical:

$$
C_c(H_1) = C_c(H_2), \qquad \mathbf{A}_{a,c}(H_1) \neq \mathbf{A}_{a,c}(H_2)
$$

— $H_2$'s $A_{\mathrm{tree}}$ cost is strictly higher and its $U_a$ reuse
gain is zero, while $H_1$'s is positive, even though nothing observed
under $c$ distinguishes them.

**Counterexample 3 — observational simplification through historical
growth.** Let $H$ be a history whose observation under $c$ distinguishes
three elements, $D_c(H) = \{(a,b),(a,c),(b,c)\}$. Append a single
`Collapse` event $e$ whose rule additionally identifies $a \sim b$:

$$
L(H \cdot e) > L(H), \qquad |D_c(H \cdot e)| < |D_c(H)|
$$

($D_c(H\cdot e)$ drops the $(a,b)$ pair). This is a concrete witness for
§6's claim: historical growth (one more event, strictly longer $L$) is
compatible with, and here directly produces, observational
simplification (strictly fewer distinguished pairs).

## 11. Relationship to `identity_after_collapse.tex` (external, not modified)

The user has proposed expanding `standardgalactic/calculus`'s
`identity_after_collapse.tex` (see `EXTERNAL-SPHEREPOP-REFERENCES.md` —
verified in this session to define real `\pop`/`\refuse`/`\bind`/
`\collapseop` macros) with a new section, "Assembly Order and Historical
Complexity," placed after that paper's primitive calculus and before its
categorical/interpretive conclusions. That repository is outside the
scope of `standardgalactic/spherepop` and has not been modified from this
session; the proposal is recorded here as reference only. At a high
level, that proposed section covers the same ground as §4, §5, §7–§10
above (collapse-relative minimal assembly, the assembly preorder and its
quotient, the `Bind`-derived dependency graph, tree-vs-DAG reuse, and a
worked example), but is meant to carry the *philosophical* framing — why
this distinction matters for what "identity after collapse" means — while
this document carries the *precise, checkable* definitions. Anyone
picking up that proposal should treat this document as the source of
truth for the formal claims and use the paper only for the interpretive
argument built on top of them, to avoid the two documents' formalism
drifting apart.

## 12. Comparison with the possibility functional $\Pi$

`Spherepop_Specifications.tex` (verified present in this session; see
`IMPLEMENTATIONS.md`'s "Canonical basis") states a conservation law:

$$
\Pi(H,\Omega) = |\Omega| + \sum_{e \in H} w(e), \qquad
\Pi(H_t,\Omega_t) = |\Omega_0| \text{ for all } t.
$$

This deserves its own scrutiny rather than an assumed compatibility with
§8's profile: is $\Pi$ intended as a conservation law, an accounting
identity, or a complexity measure? Consider the per-event change

$$
\Delta\Pi(e) = \Delta|\Omega| + w(e)
$$

for each primitive separately:

- **`Pop`:** removes exactly one option from $\Omega$ ($\Delta|\Omega| =
  -1$) and, under the specification's own weighting $w(\mathrm{Pop}) = 1$
  (`Spherepop_Specifications.tex` §"conservation," verified: "$w(\Pop(x))=1$"),
  contributes $\Delta\Pi = -1 + 1 = 0$. Conserved, exactly, for `Pop`.
- **`Refuse`:** per §8's $\Delta_{\mathrm{Refuse}}|\Omega| = 0$ (refusal
  does not shrink the option space, only records inadmissibility) and the
  specification's own weighting $w(\mathrm{Refuse}) = 0$ (verified:
  "$w(\RefuseOp)=w(\BindOp)=w(\CollapseOp)=0$"), $\Delta\Pi = 0 + 0 = 0$.
  Conserved.
- **`Bind`, `Collapse`:** likewise weighted zero by the same specification
  clause, and neither changes $|\Omega|$ (§8), so $\Delta\Pi = 0$ for
  both.

**Conclusion of this comparison:** $\Pi$ is conserved *exactly because*
`Spherepop_Specifications.tex` assigns weight $1$ to `Pop` alone and
weight $0$ to the other three primitives — it is not a universal
invariant that would hold under an arbitrary positive weighting $w$ (the
kind §2's $L_w$ allows for other purposes, such as costing `Bind` or
`Collapse` by their computational expense). Read as a claim about *that
specific* weight assignment, $\Pi$ is a genuine, exactly-derived
conservation law, matching the specification's own theorem. It should
**not** be read as evidence that $L_w$ under a general positive weight
function is conserved — under such a weighting, $\Pi$-with-general-$w$
would strictly increase on `Refuse`/`Bind`/`Collapse`, since those no
longer contribute zero. This resolves (rather than merely repeats) the
open question: the conservation claim holds in its own specified domain
and does not generalize to this document's more general $L_w$ without
adopting the same zero-weighting choice.

## 13. The OS kernel-state tuple: two projections, not one crosswalk

An earlier draft of this section proposed a tentative, coordinate-by-
coordinate correspondence between `spherepop-os.tex`'s kernel state and
this document's causal-history quantities. That was the wrong shape of
claim to make, and it is retracted here rather than merely hedged
further. The corrected conclusion is stronger than "tentative": **there
is presently no componentwise crosswalk to make**, because the two tuples
live at different layers and are not populated from the same event
vocabulary.

`spherepop-os.tex`'s kernel state (verified in this session, Definition
"Kernel State," §5.1) is

$$
\sigma = (O, U, R, M)
$$

where, exactly as defined there: $O$ is a finite set of *object
identifiers* created by `POP`; $U : O \to O$ is a *union-find parent map*
representing identity classes, updated by `MERGE`; $R \subseteq O \times
O \times \mathcal{T}$ is a multiset of *typed relations* produced by
operations such as `LINK`; and $M$ is a partial metadata map, confirmed
structurally separate from $(O,U,R)$ (`spherepop-os.tex` line 588). This
tuple belongs entirely to the **OS operational layer**.

By contrast, this document's causal-history quantities

$$
(\Omega_H, F_H, B_H, Q_H)
$$

(§1) belong to the **causal option-space layer**: possibility, refusal,
dependency, and observation. The letters $U$/$\Omega$ and $R$/$F$ are
coincidentally similar-looking, but the underlying sets are not the same
and no claim of equality should be inferred from the notation:

- $\Omega_H \neq U$. $U$ is a union-find map over objects that **already
  exist** (post-`POP`); $\Omega_H$ is the space of options that **do not
  yet exist**. These are not even subsets of the same universe.
- $F_H \neq R$. $R$ records typed relations produced by OS operations
  like `LINK`; it is not a refusal ledger. No top-level kernel event
  (`POP`, `MERGE`, `LINK`, `UNLINK`, `SET_META`) is itself a `REFUSE`, so
  there is no *kernel-event* directly analogous to causal `Refuse`.
  **Refined below**, however: causal `Refuse` is used *internally* inside
  the `LINK` reduction, and — per the per-object State term found in
  this session (see "A concrete, partial $\mathcal{I}$," below) — it
  *does* leave a trace, just not one that survives into $\sigma$.
- $B_H$/$G_H^{\mathrm{bind}}$ (§7) may be **related to** $R$ if some of
  $R$'s typed relations happen to record causal `Bind` events, but this
  requires an explicit translation rule, not a bare identification of the
  two symbols. **Refined below**: the `LINK` reduction gives exactly this
  translation rule for one specific case.

The document therefore describes two **projections from an authoritative
history**, rather than one shared coordinate space:

$$
S_{\mathrm{kernel}}(H) = (O_H, U_H, R_H, M_H),
\qquad
S_{\mathrm{causal}}(H) = (\Omega_H, F_H, B_H, Q_H).
$$

Neither is presumed recoverable from the other:

$$
S_{\mathrm{kernel}}(H) \not\Rightarrow S_{\mathrm{causal}}(H),
\qquad
S_{\mathrm{causal}}(H) \not\Rightarrow S_{\mathrm{kernel}}(H).
$$

> The causal calculus and the OS kernel expose different state spaces.
> The causal model tracks possibility, refusal, dependency, and
> observation. The OS model tracks object existence, identity classes,
> typed relations, and metadata. Although OS events may be reducible to
> expressions in the causal calculus, this does not imply a componentwise
> correspondence between their replayed states.

If a bridge between the two layers is wanted, it should be **typed as an
explicit interpretation**, not asserted by matching field names:

$$
\mathcal{I} : \mathsf{History}_{\mathrm{OS}} \longrightarrow
\mathsf{History}_{\mathrm{causal}},
$$

together with the two independent replay functions

$$
\operatorname{replay}_{\mathrm{OS}}(H) = (O, U, R, M),
\qquad
\operatorname{replay}_{\mathrm{causal}}(\mathcal{I}(H)) = (\Omega, F, B, Q).
$$

Even if $\mathcal{I}$ reduces `LINK` to a causal expression involving
`Bind`, that alone does not justify identifying $R$ with $B$: one is an
OS relation store, the other a causal dependency structure, and their
relationship is mediated entirely by $\mathcal{I}$. Likewise, since
`REFUSE` has no direct OS event, a refusal arising during reduction may
be internal to the causal derivation without surviving as a separately
queryable OS-state component under $\mathcal{I}$.

### A concrete, partial $\mathcal{I}$: found, not merely hypothesized

The previous version of this document treated $\mathcal{I} :
\mathsf{History}_{\mathrm{OS}} \to \mathsf{History}_{\mathrm{causal}}$ as
a future obligation and flagged `spherepop-os.tex` line 585's reference
to a "higher-level `State` model...defined over an option space and a
history" as not yet located. It has now been located and read in full
(Definition "State Term, Extended," `spherepop-os.tex` line 907, §9.1),
and it changes §13's picture from purely negative to partially
constructive.

The per-object causal state is literally a triple

$$
t = (I_t, \mathrm{Opt}_t, H_t)
$$

— immutable identity, admissible option space, and append-only history —
with the three primitives acting exactly as this document's §1 assumed,
but with one asymmetry worth recording precisely:

$$
\mathrm{Bind}_f(t) = (I_t,\ \{o \in \mathrm{Opt}_t : f(o)\},\ H_t),
$$

$$
\mathrm{Pop}_a(t) = (I_t,\ \mathrm{Opt}_t \setminus \{a\},\ H_t \cdot [\mathrm{sel}\!:\!a]),
$$

$$
\mathrm{Refuse}_a(t) = (I_t,\ \mathrm{Opt}_t \setminus \{a\},\ H_t \cdot [\mathrm{rej}\!:\!a]).
$$

**`Bind` does not append to $H_t$ at all** — it only filters $\mathrm{Opt}_t$.
Only `Pop` and `Refuse` leave a mark in the history component, tagged
`sel:`/`rej:` respectively. This sharpens §7's `Bind`-vs-assembly-
interpretation gap: in this concrete model, $F_H$ *is* recoverable from
$H_t$ by filtering for `rej:` tags (contrary to what the previous
paragraph's "no demonstrated storage location for refusal bookkeeping"
suggested about $\sigma$ specifically — see below for the distinction),
but $G_H^{\mathrm{bind}}$ is **not** recoverable from $H_t$ at all, since
`Bind`'s filter predicate $f$ leaves no residue once applied. Recovering
which `Bind` calls occurred, and with which predicates, requires the full
external operation sequence, not the state triple $t$ alone.

Moreover, `LINK` and kernel `COLLAPSE` are not merely claimed reducible
to the causal calculus — they have **proven reductions** already in the
text:

$$
\mathrm{LINK}(a,b,r) :=
\mathrm{Collapse}\Big(\mathrm{Refuse}_{r'}\big(\mathrm{Pop}_{r(a,b)}\big(
\mathrm{Bind}_{C_r}(H_a \otimes H_b)\big)\big)\Big)
$$

(Definition "LINK Reduction," line 505, proven well-typed at
Proposition "LINK Well-Typedness," line 517), and kernel
$\mathrm{COLLAPSE}(S, o_r)$ reduces to
$\mathrm{Collapse}\big(\bigotimes_{o \in S} H_o\big)$ together with a
*separately stated* identity-designation step (line 487). **This is a
genuine, checkable instance of $\mathcal{I}$** for two of the five kernel
events, given directly in terms of the per-object histories $H_a$, $H_b$.

This still does not rehabilitate the coordinate-by-coordinate
identification the previous section rejects, and in fact sharpens why it
must be rejected on the $U$ side specifically: `spherepop-os.tex` line 490
states explicitly that $\bigotimes_{o \in S} H_o$'s causal `Collapse`
value "does not account for $U'$'s reassignment of every $o \in S$'s
representative to $o_r$," because tensor and the causal `Collapse`
operator's own definition are silent on identity (Proposition
"tensor-structural"), and choosing $o_r$ is "an external policy decision"
layered on top of, not derived from, the causal reduction. So the earlier
draft's tentative lead — "$U$ is structurally close to a
`Collapse`-quotient" — is now **actively contradicted** by the source,
not merely unconfirmed: $U$'s update is stipulated by a policy external
to `Collapse`, so $U$ cannot be identified with a `Collapse`-induced
$\sim_c$ equivalence class structure (§5) without that external policy
coinciding with $\sim_c$ by further, separate assumption.

Finally, note that $H_a$, $H_b$, $H_o$ — the per-object causal histories
that `LINK` and `COLLAPSE` reduce over, and that would need to be
consulted to recover $F_H$ or $\Omega_H$ for a given object — are **not
components of the persisted kernel state** $\sigma = (O,U,R,M)$
(Definition "Kernel State," line 131, has exactly four fields, none of
which is a history). They are used in the reduction's *derivation* to
prove `LINK`/`COLLAPSE` well-typed, not retained afterward as queryable
state. So even though $F_H$ is recoverable in principle from $H_t$ (via
`rej:` tags), it is **not recoverable from $\sigma$** — confirming, more
precisely than before, exactly where the causal option-space/refusal
layer would need its own additional storage if an implementation wanted
to expose it, rather than treating $\sigma$ as already containing it
under a different name.

**Net conclusion, revised.** No *componentwise* crosswalk between
$\sigma = (O,U,R,M)$ and this document's $(\Omega_H, F_H, B_H, Q_H)$ is
asserted — that part of the earlier conclusion stands. What has changed
is that $\mathcal{I}$ is no longer purely hypothetical: `LINK` and kernel
`COLLAPSE` give **two concrete, proven instances** of it, expressed over
per-object causal State terms $t = (I_t, \mathrm{Opt}_t, H_t)$ that are
themselves now fully located and defined (`spherepop-os.tex` line 907).
But those per-object histories $H_t$ are consumed only inside the
reduction's *derivation*; they are not retained as fields of $\sigma$, so
$F_H$, $\Omega_H$, and $B_H$ remain unrecoverable from $\sigma$ **even
though** they are well-defined and (for $F_H$, partially for $\Omega_H$)
recoverable from the richer per-object $H_t$ that the derivation
consults. The one specific lead that does not survive is "$U$ resembles
a `Collapse`-quotient" — the source states the opposite: $U$'s update is
an external policy decision the causal `Collapse` operator's own
definition is silent on. A state coordinate still cannot automatically
substitute for a historical measure: different histories can replay to
observationally equivalent states, so $\mathbf{A}_{a,c}(H)$ is not
generally recoverable from $\sigma$ alone. What remains open is whether
`spherepop-os.tex`'s per-object $H_t$ construction is intended as the
literal implementation-level home for $\Omega_H$/$F_H$/$B_H$, or purely a
proof device for the well-typedness propositions — the text does not say
which, and this document does not resolve it.

## 14. What adoption required: a non-authoritative analysis function (implemented)

A minimal implementation need not compute every entry of $\mathbf{A}_{a,c}$
described above. It exposes a single derived, **non-authoritative**
analysis function:

$$
\operatorname{profile}_{a,c}(H) \;\longrightarrow\; \mathbf{A}_{a,c}(H).
$$

Because the profile is derived, it **must not** affect event admission,
replay, or authoritative history — exactly the same non-interference
constraint `spherepop-os.tex` already states for diffs ("Diffs do not
influence kernel state and may be dropped, reordered, or ignored by
observers without affecting correctness," line 353) and that
`spherepop-kernel`'s `Collapse` implementation already satisfies (a pure
function over `History` that never mutates state — see
`IMPLEMENTATIONS.md`'s "Canonical basis" entry for `spherepop-kernel/`).

**This is no longer a proposal.** `experiments/flat/profile.py` implements
$L$, the default assembly interpretation $a$ (`default_assembly_graph`,
$D_a$, $W_a$, $N_a$, $B_a$), the tree-vs-DAG reuse comparison
($A_{\mathrm{tree}}$, $A_{\mathrm{DAG}}$, $G_{\mathrm{reuse}}$ from §9),
and Collapse-relative minimal assembly over a supplied candidate list
(`min_length_achieving`, §4). It is kept in its own file, imports
`run_python.py`'s `Event`/`Arbiter`/collapse-rule implementations rather
than duplicating them, and is exercised by `experiments/flat/run_profile.py`
against fixtures in the isolated `experiments/flat/profile_fixtures/`
directory — deliberately separate from `experiments/flat/fixtures/` so
this non-authoritative suite cannot perturb the existing 13-fixture
conformance suite in any way. All five fixtures below pass, and the
original conformance suite (`run_python.py` against `fixtures/`) was
re-run afterward and is unaffected:

1. `13_length_increases.json` — $L(H \cdot e) = L(H) + 1$ for every
   admissible $e$ (§2), checked after each of four different event kinds
   (`pop`, `bind`, `refuse`, `collapse`).
2. `14_collapse_non_mutating.json` — `Collapse` does not mutate prior
   history: two branches share an identical prefix and diverge only in
   which certified rule their final `Collapse` invokes; the prefix's
   `option_space`, `committed` set, and `history_len` are checked
   identical across both branches.
3. `15_depth_vs_width.json` — Counterexample 1 (§10) instantiated
   exactly: a 4-object sequential chain ($D=4$, $W=1$, via exact
   Dilworth/Mirsky-duality max-antichain computation) versus 4
   independent objects with no binds ($D=1$, $W=4$), equal event/vertex
   count in both.
4. `16_reuse_tree_vs_dag.json` — Counterexample 2 (§10): a history
   reusing one shared component ($u$) across three downstream assemblies
   gives a strictly positive $G_{\mathrm{reuse}}$ (tree cost 12.0 vs. DAG
   cost 7.5), contrasted in the same fixture with a fully independent
   reconstruction of the same shape three times over, which correctly
   yields a non-positive $G_{\mathrm{reuse}}$ (tree cost 12.0 vs. DAG cost
   14.7) — demonstrating both the reuse-gain case §9 motivates and the
   documented no-reuse edge case where $G_{\mathrm{reuse}} \le 0$ is
   expected, not a defect.
5. `17_collapse_relative_minimal_assembly.json` — a **strict** witness
   for §4's proposition: two candidate histories relating objects 1 and 3
   through an intermediate object 2, then withdrawing the (1,2) relation.
   Under the coarse rule (ignores withdrawals) the shorter, length-3
   candidate already achieves "1 and 3 same class"; under the fine rule
   (honors withdrawals) only the length-4 candidate does, giving
   $A_{\min}^{\mathrm{fine}}(x) = 4 > A_{\min}^{\mathrm{coarse}}(x) = 3$ —
   a strict inequality, not just the non-strict bound proved in §4, at
   the same standard already applied to the Meld schema in
   `experiments/flat/fixtures/08_meld.json`.

## Summary

Spherepop offers no single inherent "complexity." It supplies an
authoritative history from which several invariant and policy-relative
complexity measures can be derived (§8's invariant core vs. its
$a$/$c$-dependent coordinates), while requiring every loss of distinction
(a `Collapse` rule $c$) and every scalarization (a weighting $\phi$) to
declare the rule under which it is performed. This document's two central
results are: minimal assembly complexity is monotone in observational
coarseness (§4), and historical growth is compatible with — indeed can
directly produce — observational simplification (§6, witnessed
concretely in §10's Counterexample 3). Everything else here (§7's
`Bind`/assembly-interpretation gap, §9's tree-vs-DAG reuse treatment,
§12's resolution of the possibility-functional conservation claim, §13's
separation of the OS kernel-state projection from the causal-history
projection, with two of its five kernel events now shown to have concrete
proven reductions) exists to make those two results, and any future
extension of them, precise enough to falsify. §4's monotonicity claim and
§9's tree-vs-DAG reuse gain are no longer only proved on paper: §14's
`experiments/flat/profile.py`/`run_profile.py` compute them, and all five
fixtures in `experiments/flat/profile_fixtures/` — including a strict
witness for §4's proposition and both the positive- and non-positive-gain
cases for §9's reuse comparison — pass against that implementation,
without altering the existing 13-fixture kernel conformance suite.
