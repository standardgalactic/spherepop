"""A reference interpreter for the core fragment of SpherePOP.

Terms are either values or circles.  A circle is an operator name together
with a tuple of argument terms.  Popping replaces a circle whose arguments
are all values, and whose argument tuple is admissible, by the value that
the operator produces.
"""

from dataclasses import dataclass
from typing import Callable, Dict, Optional, Tuple, Union


# ---------------------------------------------------------------- values

@dataclass(frozen=True)
class Color:
    name: str


@dataclass(frozen=True)
class Sym:
    name: str


@dataclass(frozen=True)
class Image:
    rows: Tuple[Tuple[str, ...], ...]       # a rectangular grid of colour names


Value = Union[int, Color, Sym, Image]


@dataclass(frozen=True)
class Circle:
    op: str
    args: Tuple["Term", ...]


Term = Union[Value, Circle]


def sort_of(v: Value) -> str:
    if isinstance(v, bool):
        raise TypeError("booleans are not values of the core fragment")
    if isinstance(v, int):
        return "Num"
    if isinstance(v, Color):
        return "Color"
    if isinstance(v, Sym):
        return "Sym"
    if isinstance(v, Image):
        return "Image"
    raise TypeError(f"not a value: {v!r}")


def is_value(t: Term) -> bool:
    return not isinstance(t, Circle)


# ------------------------------------------------------------- operators

@dataclass(frozen=True)
class Operator:
    name: str
    signature: Tuple[str, ...]
    result: str
    delta: Callable[..., Value]


def _rotate90(img: Image, quarter_turns: int) -> Image:
    rows = img.rows
    for _ in range(quarter_turns % 4):
        rows = tuple(zip(*rows[::-1]))          # clockwise quarter turn
    return Image(tuple(tuple(r) for r in rows))


def _rotate(img: Image, degrees: int) -> Image:
    if degrees % 90 != 0:
        raise ValueError("rotation is defined for multiples of 90 only")
    return _rotate90(img, degrees // 90)


def _recolor(img: Image, c: Color) -> Image:
    return Image(tuple(tuple(c.name if p != "." else "." for p in r)
                       for r in img.rows))


def _beside(a: Image, b: Image) -> Image:
    if len(a.rows) != len(b.rows):
        raise ValueError("images must have equal height")
    return Image(tuple(ra + rb for ra, rb in zip(a.rows, b.rows)))


OPERATORS: Dict[str, Operator] = {}


def define(name, signature, result, delta):
    OPERATORS[name] = Operator(name, tuple(signature), result, delta)


define("add", ("Num", "Num"), "Num", lambda a, b: a + b)
define("sub", ("Num", "Num"), "Num", lambda a, b: a - b)
define("mul", ("Num", "Num"), "Num", lambda a, b: a * b)
define("pow", ("Num", "Num"), "Num", lambda a, b: a ** b)
define("rotate", ("Image", "Num"), "Image", _rotate)
define("recolor", ("Image", "Color"), "Image", _recolor)
define("beside", ("Image", "Image"), "Image", _beside)


def admissible(op: Operator, args: Tuple[Value, ...]) -> bool:
    """Sort check plus the operator's own side conditions."""
    if len(args) != len(op.signature):
        return False
    if any(sort_of(a) != s for a, s in zip(args, op.signature)):
        return False
    if op.name == "pow" and args[1] < 0:
        return False                          # keep the carrier integral
    if op.name == "rotate" and args[1] % 90 != 0:
        return False
    if op.name == "beside" and len(args[0].rows) != len(args[1].rows):
        return False
    return True


# ------------------------------------------------------------- reduction

def step(t: Term) -> Optional[Term]:
    """One leftmost pop, or None if the term is in normal form."""
    if not isinstance(t, Circle):
        return None
    for i, a in enumerate(t.args):
        r = step(a)
        if r is not None:
            return Circle(t.op, t.args[:i] + (r,) + t.args[i + 1:])
    if all(is_value(a) for a in t.args):
        op = OPERATORS.get(t.op)
        if op is not None and admissible(op, t.args):
            return op.delta(*t.args)
    return None


def normalize(t: Term, limit: int = 10_000) -> Tuple[Term, int]:
    n = 0
    while n < limit:
        r = step(t)
        if r is None:
            return t, n
        t, n = r, n + 1
    raise RuntimeError("step limit reached")


def size(t: Term) -> int:
    """Number of circles in a term."""
    if not isinstance(t, Circle):
        return 0
    return 1 + sum(size(a) for a in t.args)


# ------------------------------------------------------------ substitution

def substitute(t: Term, env: Dict[str, Value]) -> Term:
    if isinstance(t, Sym):
        return env.get(t.name, t)
    if isinstance(t, Circle):
        return Circle(t.op, tuple(substitute(a, env) for a in t.args))
    return t


# --------------------------------------------------------------- printing

def show(t: Term) -> str:
    if isinstance(t, Circle):
        return "<" + " ".join([t.op] + [show(a) for a in t.args]) + ">"
    if isinstance(t, Sym):
        return t.name
    if isinstance(t, Color):
        return t.name
    if isinstance(t, Image):
        return "/".join("".join(p for p in r) for r in t.rows)
    return str(t)


# --------------------------------------------------------------- rewriting

def rewrite_step(t: Term) -> Optional[Term]:
    """Symbolic simplification: operators may now return circles."""
    if not isinstance(t, Circle):
        return None
    for i, a in enumerate(t.args):
        r = rewrite_step(a)
        if r is not None:
            return Circle(t.op, t.args[:i] + (r,) + t.args[i + 1:])
    a = t.args
    if t.op == "pow" and len(a) == 2:
        if a[1] == 1:
            return a[0]
        if a[1] == 0:
            return 1
    if t.op == "add" and len(a) == 2:
        if a[1] == 0:
            return a[0]
        if a[0] == 0:
            return a[1]
    if t.op == "mul" and len(a) == 2:
        if a[1] == 1:
            return a[0]
        if a[0] == 1:
            return a[1]
        if a[1] == 0 or a[0] == 0:
            return 0
    if all(is_value(x) for x in a):
        op = OPERATORS.get(t.op)
        if op is not None and admissible(op, a):
            return op.delta(*a)
    return None


def simplify(t: Term, limit: int = 10_000) -> Tuple[Term, int]:
    n = 0
    while n < limit:
        r = rewrite_step(t)
        if r is None:
            return t, n
        t, n = r, n + 1
    raise RuntimeError("step limit reached")


# ------------------------------------------------------------------ tests

def C(op, *args):
    return Circle(op, tuple(args))


def tower(n: int, base: Term) -> Term:
    t = base
    for _ in range(n - 1):
        t = C("pow", base, t)
    return t


if __name__ == "__main__":
    print("-- arithmetic")
    e = C("add", 2, C("mul", 3, 4))
    print(show(e), "=>", show(normalize(e)[0]), "in", normalize(e)[1], "steps")

    print("-- towers over the symbol Meta")
    meta = Sym("Meta")
    for n in range(1, 5):
        t = tower(n, meta)
        print(n, show(t), "circles:", size(t), "normal form unchanged:",
              normalize(t)[0] == t)
    print("-- towers with Meta := 2")
    for n in range(1, 5):
        t = substitute(tower(n, meta), {"Meta": 2})
        v, k = normalize(t)
        print(n, show(t), "=>", v, "in", k, "steps")
    v5 = normalize(substitute(tower(5, meta), {"Meta": 2}))[0]
    print("5 value is 2**65536:", v5 == 2 ** 65536,
          "bit length:", v5.bit_length())

    print("-- left nesting of four twos")
    left = C("pow", C("pow", C("pow", 2, 2), 2), 2)
    print(show(left), "=>", normalize(left)[0])

    print("-- images")
    I = Image((("x", ".", "x"), (".", "x", ".")))
    J = Image((("x", "x"), ("x", "x")))
    t1 = C("rotate", C("recolor", I, Color("red")), 90)
    print(show(t1), "=>", show(normalize(t1)[0]))
    t2 = C("beside", C("rotate", J, 90), C("recolor", J, Color("blue")))
    print(show(t2), "=>", show(normalize(t2)[0]))

    print("-- inert circles")
    bad = C("rotate", Color("red"), I)
    print(show(bad), "=>", show(normalize(bad)[0]), "after",
          normalize(bad)[1], "steps")
    bad2 = C("add", 1, C("pow", 2, -1))
    print(show(bad2), "=>", show(normalize(bad2)[0]), "after",
          normalize(bad2)[1], "steps")

    print("-- order independence on a small term")
    t = C("add", C("mul", 2, 3), C("mul", 4, 5))
    print(show(t), "=>", normalize(t)[0])

    print("-- rewriting")
    x = Sym("x")
    for t in (C("pow", x, 1), C("add", C("mul", x, 1), 0),
              C("mul", C("add", x, 0), 0), C("pow", C("pow", x, 1), 0)):
        s, k = simplify(t)
        print(show(t), "=>", show(s), "in", k, "steps")
