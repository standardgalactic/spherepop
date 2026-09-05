#!/usr/bin/env python3
"""Non-authoritative derived analysis functions over History, per
`COMPLEXITY.md` sections 2, 8, 9, and 14.

**These functions are derived analysis, not kernel semantics.** They must
not affect event admission, replay, or authoritative history -- the same
non-interference constraint `spherepop-os.tex` already states for Diffs
("Diffs do not influence kernel state and may be dropped, reordered, or
ignored by observers without affecting correctness," line 353) and that
`Collapse` itself already satisfies as a pure function over `History`.
This module never mutates an `Arbiter`'s history; it only reads the
already-admitted event list an `Arbiter` produced.

It deliberately imports `run_python`'s `Event`/`Arbiter`/collapse-rule
implementations rather than reimplementing them -- duplicating the oracle
would itself be an instance of the tree-vs-DAG reuse distinction this
module exists to measure (`COMPLEXITY.md` section 9).

Every quantity below names the policy it depends on, per `COMPLEXITY.md`'s
central discipline:

- `L` is policy-free (section 2).
- `default_assembly_graph`/`depth`/`width`/`num_components`/
  `num_dependencies` all depend on one named assembly interpretation,
  `"default"` (section 7): a `bind(a, b, tag)` event with
  `tag != "__meta__"` is read as "b depends on a," i.e. an edge a -> b.
  This is one possible interpretation `a`, not the only one; a
  different interpretation could read the same `Bind` events completely
  differently, which is exactly section 7's point.
- `tree_cost`/`dag_cost`/`reuse_gain` additionally depend on a weight
  function `w` (default: 1 per vertex) and a reuse-cost function
  `lam` (default: 0.3 per dependency edge, chosen `< w` per
  `COMPLEXITY.md` section 9's "usually lambda_K < A(K)").
- `min_length_achieving` depends on a named collapse rule (`"coarse"` or
  `"fine"`, both already implemented in `run_python.py`) and searches
  only over an explicitly supplied candidate list -- it is not a general
  minimal-history synthesis engine. Searching over *all* admissible
  histories of a given target is combinatorially unbounded and out of
  scope for a derived, non-authoritative analysis function; fixtures
  must supply the candidates whose costs are to be compared.
"""
from __future__ import annotations

import sys
from pathlib import Path
from typing import Callable, Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
from run_python import (  # noqa: E402
    Arbiter,
    Event,
    collapse_quotient,
    collapse_quotient_honoring_refusals,
    events_from_op,
)

# ---------------------------------------------------------------------
# Section 2: L(H) = |H|. Policy-free.
# ---------------------------------------------------------------------


def event_length(history: list[Event]) -> int:
    """L(H) = |H| (`COMPLEXITY.md` section 2)."""
    return len(history)


# ---------------------------------------------------------------------
# Section 7/8: default assembly interpretation `a`.
# ---------------------------------------------------------------------


def default_assembly_graph(history: list[Event]) -> tuple[set[int], set[tuple[int, int]]]:
    """G_H^assembly under the named interpretation "default": vertices are
    every object introduced by `pop`; edges are (a, b) for each
    `bind(a, b, tag)` with `tag != "__meta__"`, read as "b depends on a."
    This is a policy choice (`COMPLEXITY.md` section 7), not the only
    legitimate reading of `Bind`."""
    vertices: set[int] = set()
    edges: set[tuple[int, int]] = set()
    for e in history:
        if e.kind == "pop":
            vertices.add(e.a)
        elif e.kind == "bind" and e.tag != "__meta__":
            vertices.add(e.a)
            vertices.add(e.b)
            edges.add((e.a, e.b))
    return vertices, edges


def _parents(edges: set[tuple[int, int]]) -> dict[int, list[int]]:
    parents: dict[int, list[int]] = {}
    for (u, v) in edges:
        parents.setdefault(v, []).append(u)
    return parents


def depth(history: list[Event]) -> int:
    """D_a(H): longest dependency path, counted in vertices, under the
    default assembly interpretation (`COMPLEXITY.md` section 8)."""
    vertices, edges = default_assembly_graph(history)
    if not vertices:
        return 0
    parents = _parents(edges)
    memo: dict[int, int] = {}

    def longest_path(v: int) -> int:
        if v in memo:
            return memo[v]
        ps = parents.get(v, [])
        result = 1 + max((longest_path(p) for p in ps), default=0)
        memo[v] = result
        return result

    return max(longest_path(v) for v in vertices)


def width(history: list[Event]) -> int:
    """W_a(H): the exact maximum antichain size in G_H^assembly, computed
    via Dilworth/Mirsky duality: max antichain size = |V| - (maximum
    bipartite matching over the reachability relation). This is an exact
    computation, not an approximation, and is tractable for the small
    graphs a derived analysis function is expected to see."""
    vertices, edges = default_assembly_graph(history)
    verts = sorted(vertices)
    n = len(verts)
    if n == 0:
        return 0
    idx = {v: i for i, v in enumerate(verts)}
    adj: list[list[int]] = [[] for _ in range(n)]
    for (u, v) in edges:
        adj[idx[u]].append(idx[v])

    reach = [[False] * n for _ in range(n)]
    for i in range(n):
        stack = list(adj[i])
        seen: set[int] = set()
        while stack:
            x = stack.pop()
            if x in seen:
                continue
            seen.add(x)
            stack.extend(adj[x])
        for j in seen:
            reach[i][j] = True

    match_right = [-1] * n

    def try_augment(u: int, visited: list[bool]) -> bool:
        for v in range(n):
            if reach[u][v] and not visited[v]:
                visited[v] = True
                if match_right[v] == -1 or try_augment(match_right[v], visited):
                    match_right[v] = u
                    return True
        return False

    matching = 0
    for u in range(n):
        visited = [False] * n
        if try_augment(u, visited):
            matching += 1

    min_chain_cover = n - matching
    return min_chain_cover


def num_components(history: list[Event]) -> int:
    """N_a(H) = |V(G_H^assembly)| under the default interpretation."""
    vertices, _ = default_assembly_graph(history)
    return len(vertices)


def num_dependencies(history: list[Event]) -> int:
    """B_a(H) = |E(G_H^assembly)| under the default interpretation."""
    _, edges = default_assembly_graph(history)
    return len(edges)


# ---------------------------------------------------------------------
# Section 9: reuse via tree-vs-DAG comparison.
# ---------------------------------------------------------------------

DEFAULT_WEIGHT = 1.0
DEFAULT_LAMBDA = 0.3


def tree_cost(
    history: list[Event],
    w: Callable[[int], float] = lambda v: DEFAULT_WEIGHT,
) -> float:
    """A_tree(H): the cost of T_H, the tree obtained by duplicating every
    reused component's entire ancestor lineage at every place it is
    consumed. Implemented as: for every sink vertex r (no outgoing edge
    in G_H^assembly), sum `ancestors_size(r)`, where
    `ancestors_size(v) = w(v) + sum(ancestors_size(p) for p in parents(v))`.
    A shared ancestor's `ancestors_size` is counted once per distinct
    downstream sink that depends on it, which is exactly the duplication
    T_H represents (`COMPLEXITY.md` section 9)."""
    vertices, edges = default_assembly_graph(history)
    if not vertices:
        return 0.0
    parents = _parents(edges)
    has_outgoing = {u for (u, _v) in edges}
    sinks = [v for v in vertices if v not in has_outgoing]
    memo: dict[int, float] = {}

    def ancestors_size(v: int) -> float:
        if v in memo:
            return memo[v]
        total = w(v) + sum(ancestors_size(p) for p in parents.get(v, []))
        memo[v] = total
        return total

    return sum(ancestors_size(r) for r in sinks)


def dag_cost(
    history: list[Event],
    w: Callable[[int], float] = lambda v: DEFAULT_WEIGHT,
    lam: Callable[[tuple[int, int]], float] = lambda e: DEFAULT_LAMBDA,
) -> float:
    """A_DAG(H) = sum_v w(v) + sum_e lambda(e) over the actual
    (non-duplicated) G_H^assembly (`COMPLEXITY.md` section 9)."""
    vertices, edges = default_assembly_graph(history)
    return sum(w(v) for v in vertices) + sum(lam(e) for e in edges)


def reuse_gain(
    history: list[Event],
    w: Callable[[int], float] = lambda v: DEFAULT_WEIGHT,
    lam: Callable[[tuple[int, int]], float] = lambda e: DEFAULT_LAMBDA,
) -> float:
    """G_reuse(H) = A_tree(H) - A_DAG(H) (`COMPLEXITY.md` section 9).

    Not asserted to be non-negative for every history: a DAG with no
    shared ancestors at all (nothing reused) yields T_H = G_H, so
    G_reuse(H) = -sum_e lambda(e) <= 0, correctly reflecting "no reuse
    occurred, and binding still had a nonzero cost" rather than a genuine
    reuse gain. The claim this document makes is narrower and still
    holds: when a history reuses a component (some vertex has more than
    one outgoing edge) and `lam < w`, the resulting gain is strictly
    positive, unlike the retracted subtractive formula, which could go
    unboundedly negative even in ordinary cases."""
    return tree_cost(history, w) - dag_cost(history, w, lam)


# ---------------------------------------------------------------------
# Section 4: Collapse-relative minimal assembly, over supplied candidates.
# ---------------------------------------------------------------------

COLLAPSE_RULES: dict[str, Callable[[list[Event]], object]] = {
    "coarse": collapse_quotient,
    "fine": collapse_quotient_honoring_refusals,
}


def observes_same_class(rule_name: str, history: list[Event], a: int, b: int) -> bool:
    """C_c(H) restricted to the question 'are a and b in the same
    observed class', for one of the two named collapse rules already
    implemented in `run_python.py`. `"fine"` (honors refusals/withdrawn
    relations) is the finer rule; `"coarse"` (ignores refusals) is
    coarser: `"fine" sqsubseteq "coarse"` in `COMPLEXITY.md` section 4's
    notation."""
    uf = COLLAPSE_RULES[rule_name](history)
    return uf.same_class(a, b)


def min_length_achieving(
    rule_name: str,
    candidates: list[list[Event]],
    target: tuple[int, int],
) -> Optional[int]:
    """A_min^c(x) restricted to a supplied candidate list: the minimum
    L(H) among `candidates` whose observation under `rule_name` puts
    `target = (a, b)` in the same class. Returns `None` if no candidate
    achieves the target. This is deliberately not a general search over
    all admissible histories (`COMPLEXITY.md` section 14)."""
    a, b = target
    lengths = [
        event_length(h)
        for h in candidates
        if observes_same_class(rule_name, h, a, b)
    ]
    return min(lengths) if lengths else None


# ---------------------------------------------------------------------
# Helper: build an admitted history from a fixture-style op list, reusing
# the Arbiter's own admissibility checks so profile computations only
# ever see genuinely admissible histories.
# ---------------------------------------------------------------------


def build_history(omega0: list[int], rules: list[str], ops: list[dict]) -> list[Event]:
    arb = Arbiter(set(omega0), set(rules))
    for op in ops:
        arb.submit(events_from_op(op))
    return arb.history
