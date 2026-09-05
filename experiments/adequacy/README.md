# Language adequacy (Phase E)

This is the Phase E deliverable from tracking issue #1: evidence at the
"Expressive" level — "translations from a small known calculus... show
what Spherepop can compute, without making computability the sole
criterion of languagehood."

## What is here

- **`lambda_calculus.md`** — the normative definition of the source
  calculus (a tiny call-by-value lambda calculus with integers,
  closures, `let`, `if0`, and one recursive-binding form), the
  translation into Spherepop's four primitive events, the read-back
  ("observation") relation, and the exact preservation claim checked
  below. Read this first.
- **`translate_lambda.py`** — the translator and an independent
  reference evaluator, plus seven example programs and a self-checking
  runner. Reuses the exact kernel from `experiments/flat/run_python.py`
  (imported, not reimplemented), so every translated program is
  validated by the same `Arbiter.submit` the flat fixture suite uses.
- **`traces/*.json`** — one checked-in, human-readable trace per
  example program: the source term, the reference evaluator's result,
  the Spherepop-observed result, and the complete ordered list of
  primitive events actually submitted and accepted.
- **`run_adequacy.sh`** — one-command runner.

## Running it

```sh
./run_adequacy.sh
```

Exits non-zero if any program's translated trace is rejected by the
Arbiter, or if the reference evaluator's result disagrees with the
Spherepop-observed result (read back via the `"result"` collapse rule,
see `lambda_calculus.md` Sec 2).

## What this demonstrates (Phase E checklist)

| Checklist item | Where |
|---|---|
| Names/bindings as a derived facility | Every function application emits an audit `Bind(site, value, "env:x")` (see `identity_application`, `composition`, `recursion_sum` traces). |
| Composition / reusable abstraction | `composition`: two closures (`double`, `inc`) each defined once, applied in sequence; `recursion_sum`: the same closure applied 5 times across recursive calls. |
| Conditional choice/refusal as a derived facility | `conditional_true`/`conditional_false`: `if0` realized exactly as the existing `Choice` sugar (`Pop(taken)`, `Refuse(rejected, ...)`), reusing `SPEC.md` §7's derived form rather than inventing a new one. |
| Iteration/recursion as a derived facility | `recursion_sum`: a genuinely self-referential closure, recursing through the *same* `App` translation case as any other call — no new event kind, no artificial fuel bound. Termination is a property of the source program, exactly as in `sum_to(4)`'s five nested calls visible in `traces/recursion_sum.json`. |
| Several small, inspectable programs | Seven programs, `traces/*.json` — the largest (`recursion_sum`) is 52 events and readable end to end by hand. |
| Translation from a small known model | `lambda_calculus.md` — a full untyped call-by-value lambda-calculus fragment. |
| Test (or prove) preservation of the observed semantics under translation | `translate_lambda.py::run_program` asserts `reference_value == observed_value` for every program; all seven pass. |

## Honest limits (not claimed here)

- This is a **test**, not a machine-checked proof, of preservation —
  consistent with the issue's own "Prove or test" wording. A general
  proof over the whole calculus (rather than seven example programs) is
  Phase F ("Formal assurance") scope and is not attempted here.
- Recursion is real (a genuinely self-referential closure, not
  translator-imposed unrolling) but every example program is chosen to
  provably terminate. No claim of unbounded recursion, Turing
  completeness, or general expressiveness beyond this fragment is made
  — see `SPEC.md` §11 and the tracking issue's explicit statement that
  computability is not the sole criterion of languagehood.
- Substitution/beta-reduction itself is performed by the translator's
  own environment (as in any interpreter), not by a Spherepop
  primitive — Spherepop's four primitives have no substitution
  operator. What is checked is that the *audit trail* the translation
  leaves behind (`Bind(site, value, "env:x")` for every binding of a
  concrete value) faithfully records what the reference evaluator
  actually does, and that the final observed result matches. See
  `lambda_calculus.md` Sec 2 for the precise scope of what "names/
  bindings as a derived facility" does and does not claim here.
