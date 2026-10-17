# Duplication and Universality in the Circle Calculus

An executable companion to the essay in `essay/duplication-and-universality.tex`.
It extends the pure circle fragment of *An Introduction to SpherePOP* (chapters 1–7,
see `experiments/circles`) by one explicit change, a binary application circle with
the rules of combinatory logic, and checks what that change costs and what it buys.

No package install, build step or network access is needed. Node.js 20+ and Python 3.10+.

```sh
node --max-old-space-size=6144 test.mjs # all groups; full corpus needs a larger heap
node report.mjs        # regenerate generated/facts.tex and generated/tables.tex
cd essay && latexmk -pdf duplication-and-universality.tex
```

The exhaustive suite retains interned terms across groups and requires substantial
memory. The command above gives Node a 6 GiB heap; the host also needs room for
the Python oracle and temporary buffers. CI uses the same heap setting. This does
not change any enumeration or reduction bounds.

## What is here

| File | Role |
| --- | --- |
| `cl.mjs` | Hash-consed S/K circles, the four sequential strategies, Gross–Knuth development, parallel reducts, an abstract machine for leftmost-outermost normal forms (tree and shared), bracket abstraction, derivations written contraction by contraction. |
| `lam.mjs` | Named lambda terms, capture-avoiding substitution, weak and full normal-order reduction, the translation into circles, a constructive simulation of weak beta steps, Church numerals. |
| `machines.mjs` | Counter machines, a reference interpreter, a compiler to lambda terms (Scott numerals, a call-by-name fixed point, a dispatch chain), decoders. |
| `dp.mjs` | Dynamic-programming views: reduction graphs with Bellman recursions, the pure fragment's pop orderings (state DP and the hook length formula), interval DP over parenthesizations. |
| `decompose.mjs` | Construction traces: centre-and-correct decompositions with exact polynomial identities. |
| `oracle.py` | An independent Python implementation (no interning, no cached flags) of the strategies, Gross–Knuth development, abstraction, translation and the machine semantics. |
| `test.mjs` | The verification suite. Writes `generated/facts.json`. |
| `combinators.workflow.yml` | GitHub Actions workflow to copy into `.github/workflows/`. |
| `report.mjs` | Turns the measured facts into LaTeX so no number in the essay is typed by hand. |

## The calculus

A term is `S`, `K`, a free variable, or an application circle `A(f, x)`, written by
juxtaposition and left-associated. The rules accept arbitrary terms as operands,
which departs from the book's requirement that operands already be values:

```
K x y     ->  x
S x y z   ->  x z (y z)
```

`I` is the abbreviation `S K K`. The pure fragment is strongly normalizing because
every pop removes one circle. Here a contraction of `K x y` removes `2 + |y|`
circles, but a contraction of `S x y z` adds `|z|`.

## What is proved, cited, checked, and not claimed

See Section 13 of the essay for the full ledger. In short:

* **Proved in the essay**: the size laws; the exact leftmost-outermost sequence on
  `W (I^n W)`; Church–Rosser by parallel reduction; the bracket abstraction theorem
  with its exact step count; the substitution lemma; simulation of weak beta steps;
  failure of reduction under a binder; simulation of counter machines; the hook
  length and interval-DP counts.
* **Cited**: leftmost normalization for orthogonal systems (Curry and Feys; O'Donnell;
  Huet and Lévy), universality of counter machines (Minsky), and the correspondence
  between head reduction of a lambda term and leftmost-outermost reduction of its
  translation, on which the non-halting direction of the universality argument depends.
* **Checked by machine, within stated bounds**: every word up to 6 leaves over
  `{S, K, x}` and every word of 7 and 8 leaves over `{S, K}` (including the first words
  with no normal form), all lambda terms up to size 6 or 7, and a differential
  comparison of 724,860 strategy-by-term runs against `oracle.py`, of which a few hundred
  end in a cycle or exhaust a budget.
* **Not claimed**: that the four-event history kernel of the repository simulates lambda
  reduction; that any particular non-halting program is proved divergent in the circle
  calculus (a budget is exhausted, and the argument is conditional on the cited
  correspondence); that the translation is an equivalence of calculi; that sharing gives an
  asymptotic law (ratios are measured); that construction traces help anyone learn;
  anything about efficiency beyond the measured tables.

## Reading the tests

Each group prints `PASS Tnn title [note]`. A failure stops the run with the
offending term. The bounds are part of each title; a skeptic can raise them.
