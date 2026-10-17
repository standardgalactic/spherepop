# Choose the Next Pop

An interactive companion to *An Introduction to SpherePOP: Nested Circles,
Typed Operators, and Reduction*, chapters 1–7. Open
[`index.html`](index.html) through a static HTTP server. No package install,
external assets, build step, or service is needed.

From the repository root:

```sh
python3 -m http.server 8000
```

Visit <http://localhost:8000/experiments/circles/>. Select an eligible circle
and pop it, reduce a simultaneous frontier, undo an action, substitute an
integer for a free symbol, or compare leftmost, rightmost and parallel
strategies. The JSON editor accepts typed terms, never executable Python or
JavaScript. Export preserves exact input and output terms for every action.

The presets cover explicit grouping, disjoint redexes, balanced parallel
reduction, partial evaluation, inert terms, power towers, immutable images,
recolouring, clockwise rotation, composition and domain restrictions.
J/K selects ready circles, Enter pops, P performs one simultaneous round,
U undoes, and R resets. Buttons and circle selection also support keyboard
focus. Editing fields do not trigger these shortcuts. The layout supports
small screens and uses no animation or audio.

## What the experiment establishes

`engine.mjs` implements pure value-returning operators using exact BigInt
arithmetic. A normal form may still be an inert circle: unknown operators,
wrong arities, unsupplied symbols, sort mismatches and inadmissible operand
tuples do not become successful values. The resource budget is reported
separately; an admissible operation can exceed browser limits.

Each ordinary pop removes exactly one circle. The simultaneous operation
captures all ready positions before modifying the term, so newly enabled
parents wait until the next round. Left/right/parallel comparisons start
from the current expression. Bounded exploration deduplicates identical
terms, counts distinct states and transitions, and explicitly reports when
its state budget prevents exhaustive exploration. It does not count all
paths and is not a proof of general confluence. Parallel rounds measure
logical dependency depth, not actual parallel execution or wall-clock time.

Input is limited to 255 terms, depth 24, 100 KB of JSON, and rectangular
images up to 16 × 16. Arithmetic has a 20,000-decimal-digit result limit
and a conservative exponent budget. The book's five-two tower, 2**65536,
fits. Display abbreviates large numbers while exported traces retain them.
The total-pops metric includes every recorded pop, including ones taken
before a substitution. The parallel-rounds metric counts only explicitly
requested simultaneous rounds, not mixed sequential actions.

This core experiment does not implement the symbolic rewriting extension
of chapter 8, lambda binding, mutable instruction bubbles, or the canonical
event-history kernel in `experiments/flat`. In particular, an inert circle
is not a REFUSE event and a pop here is not a historical commitment event.
The existing `reduction-engine/` remains a separate teaching model.

## Verification

With Node.js 20+ and Python 3.10+ available, run from the repository root:

```sh
node experiments/circles/test.mjs
```

The runner uses `python3` by default. Set `PYTHON` to another executable
name or path when needed, for example `PYTHON=python node
experiments/circles/test.mjs` on a POSIX shell, or `$env:PYTHON = "python"`
before running the command in PowerShell. `PYTHON=py` also works with the
Windows Python launcher; the setting names one executable, not a shell command.

Optional browser regressions use Playwright and Chromium. After installing
those development tools, run `node experiments/circles/test-browser.mjs`.
The script starts its own temporary local server and checks letter shortcuts
with button focus, native Enter activation, substitution draft preservation,
editing, images, export and mobile containment. `PLAYWRIGHT_MODULE` can name
an alternate module location, and `CHROMIUM_PATH` an existing browser binary.
These browser checks are separate from the dependency-free reference CI job.

`oracle.py` explicitly loads `spherepop-book-source/spherepop.py` by path,
avoiding collisions with older packages named `spherepop`. Presets,
deterministically generated arithmetic and symbolic terms, substitutions,
invalid operator applications, images and a large integer are compared
against the book for their first reduction, normal form, size and pop count.
Independent checks exercise disjoint-pop diamonds, size decrease, strategy
agreement, parallel-frontier timing, image orientation and persistence,
exploration cutoffs and resource/input limits.

The dedicated `circle-laboratory.yml` workflow checks changes to this
experiment, the book interpreter, or its own definition. It uses sparse
checkout and read-only permissions. The existing C/cross-language workflow
and its conformance report are unaffected.
