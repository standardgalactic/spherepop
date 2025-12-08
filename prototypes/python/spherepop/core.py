from __future__ import annotations
from dataclasses import dataclass
from typing import Any, Callable, List, Protocol, Union


@dataclass
class Region:
    label: str
    payload: List[Any]

    def __repr__(self) -> str:
        return f"Region(label={self.label!r}, payload={self.payload!r})"


class CollapseStrategy(Protocol):
    def __call__(self, r: Region) -> Region: ...


def default_collapse(r: Region) -> Region:
    # For now: identity on label, flatten payload one level.
    flat: List[Any] = []
    for p in r.payload:
        if isinstance(p, list):
            flat.extend(p)
        else:
            flat.append(p)
    return Region(label=r.label, payload=flat)


def merge(strategy: CollapseStrategy, r1: Region, r2: Region) -> Region:
    union_payload: List[Any] = []
    union_payload.extend(r1.payload)
    union_payload.extend(r2.payload)
    return collapse(strategy, Region(label=f"{r1.label}+{r2.label}", payload=union_payload))


def collapse(strategy: CollapseStrategy, r: Region) -> Region:
    return strategy(r)


# --- term language ---

@dataclass
class Atom:
    r: Region


@dataclass
class Pop:
    t: "Term"


@dataclass
class Merge:
    left: "Term"
    right: "Term"


Term = Union[Atom, Pop, Merge]


def eval_term(strategy: CollapseStrategy, t: Term) -> Region:
    if isinstance(t, Atom):
        return t.r
    if isinstance(t, Pop):
        return collapse(strategy, eval_term(strategy, t.t))
    if isinstance(t, Merge):
        return merge(strategy, eval_term(strategy, t.left), eval_term(strategy, t.right))
    raise TypeError(f"Unknown term: {t!r}")
