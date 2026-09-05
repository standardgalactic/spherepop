# Translation: a tiny lambda-calculus fragment → Spherepop primitives

This is the Phase E / §3 deliverable from issue #1: "Define a translation
from a tiny Forth, lambda-calculus fragment, register machine, or
similarly small known model" and "Prove or test preservation of the
chosen observable semantics under translation." It is a companion to
`SPEC.md` (the normative core) and to `experiments/flat/` (Phase C/D);
read those first for the primitive semantics assumed here.

This document defines the source calculus, the translation function
`⟦·⟧` into traces of Spherepop's four primitive events, the read-back
("observation") relation used to recover a translated program's result,
and the exact preservation claim `translate_lambda.py` checks. Nothing
here introduces a fifth primitive event or bypasses the Arbiter's real
validation rules (§3 of `SPEC.md`) — every translated trace is executed
through the same `Arbiter.submit` that the flat fixture suite uses.

## 1. Source calculus

A small, terminating-by-construction, call-by-value lambda calculus
with integer literals, first-order and higher-order functions used
first-order in the example programs below, `let`, `if0`, arithmetic,
and one recursive-binding form:

```
e ::= n                    -- integer literal
    | x                    -- variable
    | λx. e                -- abstraction (closure)
    | e e                  -- application
    | e + e | e - e        -- arithmetic
    | if0 e then e else e  -- zero test (call-by-value: only the
                            --   taken branch is evaluated)
    | let x = e in e        -- let-binding, sugar for (λx.e2) e1
    | rec f(x) = e           -- self-referential closure: `f` is bound
                            --   to the closure inside its own body
```

Reference semantics: a standard big-step evaluator `eval(e, ρ) → v`
over integers and closures, `ρ` a variable environment. This evaluator
is the **ground truth** against which the translation is checked; it is
a plain Python function, entirely independent of the Spherepop kernel.

## 2. Translation target and conventions

The translation emits ordinary Spherepop events — `Pop`, `Refuse`,
`Bind`, `Collapse`, and the derived `Choice` (§7 of `SPEC.md`) — into a
real `Arbiter` (reusing `experiments/flat/run_python.py`'s kernel
verbatim, so this experiment is checked against the exact same
`apply`/`submit` code the flat fixtures already exercise). Three
disjoint integer ranges of `ObjectId` are used purely as a bookkeeping
convention (nothing in the kernel itself distinguishes them):

- **Value atoms** (small integers, e.g. `0..99`): each integer literal
  `n` used anywhere in a program is *represented by the object id `n`
  itself*. Value atoms are never Popped and never appear in a world's
  option space — they exist only as the second element of `Bind`
  triples (see below), which is legal because `Bind`'s well-formedness
  (`SPEC.md` §3, rule 3) never requires Ω-membership of either
  argument.
- **Result objects** (a tight, per-program range computed by a sizing
  pre-pass, see §4 below): one fresh object per literal, arithmetic
  result, or branch selection that a program actually commits.
- **Binding-site objects**: one fresh object per function application,
  used only as the first element of an audit `Bind`, never Popped.

**Committing to a value:** whenever the translation produces a
concrete integer result `n` at a fresh result object `v`, it emits
exactly:

```
Pop(v)
Bind(v, n, "denotes")
```

`Pop(v)` commits to "the object standing for this particular
computation's result"; `Bind(v, n, "denotes")` couples that result
object to the value atom `n` *without identifying them* — precisely
Bind's canonical meaning (`SPEC.md` §2). Reading a result object's
denoted integer back out is the read-back relation:

```
denote(H, v) = the unique n such that Bind(v, n, "denotes") ∈ H
```

which is exactly the same shape as `collapse_meta` in `SPEC.md` §5 (a
pure function of `H` that reads back Bind facts under a reserved tag)
— here named `collapse_result` and certified as a named rule `"result"`
in every translated world, with a `Collapse("result")` event emitted at
the end of every program's trace as an explicit, auditable observation
step. (Per Observation Non-Interference, `Collapse("result")` itself
stores only the *fact* that rule `"result"` was invoked, in
`state.observed` — never the value; `collapse_result` is called
separately, exactly like every other collapse rule in `SPEC.md` §5.)

**Names and bindings:** applying a closure with parameter `x` to an
already-translated argument object `av` emits a fresh binding-site
object `site` and:

```
Bind(site, av, "env:x")
```

purely as an audit-trail record of "in this call, `x` is bound to
`av`" — this is the derived facility the Phase E checklist calls
"names/bindings." This audit `Bind` is only emitted when the bound
value is a concrete, already-committed Spherepop object (an integer
result); a name bound directly to a not-yet-invoked function value has
no Spherepop-level audit entry, since functions are not object-space
citizens in this experiment — only the integers a call eventually
commits to are. This is a deliberate scope limitation stated here, not
silently glossed over: `composition`'s `let double = λx. ... in ...`
binding is invisible in the trace for exactly this reason, while every
call to `double` (an `App`, producing a concrete result) is fully
audited. **Actual** substitution — resolving what `x` means
inside the closure's body during translation — is done by the
translator's own variable environment (a Python dict), exactly as it
is done in the reference evaluator `eval`. This mirrors the existing,
already-checked-in honesty convention in `experiments/flat/fixtures/
11_small_arithmetic.json` ("This does not ask the kernel to perform
integer addition"): Spherepop's four primitives do not include
substitution or beta-reduction as a mechanized operation, and this
translation does not pretend otherwise. What *is* claimed, and *is*
checked, is that the audit trail left behind — the `Bind(site, av,
"env:x")` facts — faithfully records every binding the reference
evaluator actually performs, in the same order.

**Conditionals as Choice:** `if0 c then t else e` first translates and
evaluates `c` to a result object, reads back its denoted integer, and
then emits a `Choice(taken, rejected)` (`SPEC.md` §7 —
`Pop(taken)`,`Refuse(rejected, "not selected by Choice")`) over two
fresh, single-use "branch marker" objects, before translating *only*
the taken branch's body. This both (a) reuses an existing derived form
exactly as specified rather than inventing a new one, and (b) keeps
faith with call-by-value `if0`'s "only evaluate the taken branch"
requirement — the rejected branch's *body* is never translated or
evaluated, only its *marker* is auditably refused.

**Recursion:** `rec f(x) = e` translates to a translator-level closure
value whose captured environment includes itself under the name `f`
(the standard tie-the-knot construction for recursive closures, done at
the meta level in the translator, exactly as the reference evaluator
does it). Every recursive call is translated by the *same* `App` case
as any other application — no new event kind, no special-cased
"recursion" event. A program that recurses `k` times therefore leaves
behind `k` structurally identical audit sub-traces (one `Bind(site, _,
"env:x")` and one result `Pop`/`Bind(_, _, "denotes")` per call),
differing only in which objects they name — the same notion of
"genuine abstraction" already stated in
`experiments/flat/fixtures/12_small_abstraction.json`. Termination is a
property of the *source program* (each example given here provably
decreases toward its base case), not of an artificial fuel/step bound
imposed by the translator — this experiment does not claim or need
unbounded recursion or Turing-completeness; that remains explicitly out
of scope (see `SPEC.md` §11 and the "Expressive" evidence level in the
tracking issue, which is explicit that computability is not the sole
criterion of languagehood).

## 3. The preservation claim

For every example program `e` in `translate_lambda.py`:

```
eval(e, {}) = collapse_result(translate(e), root_object_of(e))
```

i.e. the reference evaluator's result and the Spherepop-observed result
(read back via the `"result"` collapse rule from the *actually
executed, Arbiter-validated* trace) are equal integers. This is checked
by direct execution (`assert`), not proved on paper — per the tracking
issue's own "Prove **or test**" wording, a test across the example
programs below is the claimed evidence, and no stronger claim is made.

Additionally, every translated trace must be **accepted** by the real
`Arbiter.submit` (no `ArbiterError`) — i.e. every translated program is
independently checked to be a well-formed sequence of primitive events
under the exact same validation rules as the flat fixture suite, not a
hand-waved "morally equivalent" sequence.

## 4. Sizing pass (why the checked-in traces have tight option spaces)

An `Arbiter`'s `Ω₀` must be fixed before any `Pop` can be validated
(`SPEC.md` §3), but the translator only discovers which objects it will
Pop as it walks the term. `translate_lambda.py` therefore translates
each program twice with an identical, reset object-id counter: once
against a permissive recorder that shares the exact same `apply`
function as the real kernel but skips Arbiter-level validation (purely
to discover the exact set of ids that will be Popped), and once for
real against a properly-sized `Arbiter` constructed from that exact
set. This is why the checked-in trace JSON files in `traces/` have
small, exact `initial_option_space` lists (e.g. eight objects for an
eight-Pop program) rather than an oversized pool — matching the flat
fixture suite's own convention of using the smallest ids that make a
trace by-hand-readable.

## 5. Example programs

See `translate_lambda.py`'s `PROGRAMS` list and `traces/*.json` for the
full, inspectable, checked-in traces. In summary:

| Program | Demonstrates |
|---|---|
| `literal` | Baseline: a single `Lit` commits and denotes its value. |
| `arithmetic` | `Add`/`Sub` via a fresh result object and a `Bind` recording the two operands (mirrors `11_small_arithmetic.json`). |
| `identity_application` | Application and the `env:x` name-binding audit trail, on the simplest possible closure. |
| `composition` | Two reusable, separately-defined first-order closures (`double`, `inc`) applied in sequence — composition and reusable abstraction. |
| `conditional_true` / `conditional_false` | `if0` taking each branch, both realized as `Choice` (`Pop`+`Refuse`) over branch markers. |
| `recursion_sum` | `rec sum(n) = if0 n then 0 else n + sum(n-1)` applied to a small `n` — genuine, terminating recursion via the same `App` case as any other call, no new event kind, no artificial fuel bound. |
