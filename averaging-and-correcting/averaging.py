#!/usr/bin/env python3
"""Verified reference decompositions of products.

For a chosen reference m, with a = m + u and b = m + v,

    a*b = m^2 + m*(u + v) + u*v.

Every equality in a trace is evaluated in exact rational arithmetic, and the
final total is compared with a product computed without using the
decomposition. A proposed transformation that fails is refused with its
residual rather than silently repaired, and the residual is compared against
the three terms to suggest which one may have been dropped or mis-signed.

Nothing here scores how hard a choice is. The exploration command reports,
for a stated repertoire of "round" quantities, which references leave every
needed term inside that repertoire. The repertoire belongs to the person
using the tool, and it is an input, not a finding.

Subcommands:
    trace   A B [--ref M]
    check   A B --claim EXPR [--ref M] [--justification TEXT] [--log FILE]
    explore A B [--lo N] [--hi N] [--halves] [--extra-known N ...] [--all]

In expressions, ^ means exponent. Only numbers, + - * /, ^ and parentheses
are accepted.
"""
from __future__ import annotations

import argparse
import ast
import itertools
import json
import math
import operator
import sys
from dataclasses import dataclass, field
from datetime import datetime, timezone
from fractions import Fraction
from pathlib import Path
from typing import Iterable, Optional, Sequence


# ---------------------------------------------------------------------------
# Numbers and formatting
# ---------------------------------------------------------------------------

def to_fraction(x) -> Fraction:
    """Convert an int, Fraction, or numeric string/float to an exact Fraction."""
    if isinstance(x, Fraction):
        return x
    if isinstance(x, bool):
        raise TypeError("booleans are not numbers here")
    if isinstance(x, int):
        return Fraction(x)
    return Fraction(str(x).strip())


def fmt(x) -> str:
    """Exact, human-readable rendering: integers, terminating decimals, or p/q."""
    x = to_fraction(x)
    if x.denominator == 1:
        return str(x.numerator)
    rest = x.denominator
    for p in (2, 5):
        while rest % p == 0:
            rest //= p
    if rest != 1:
        return f"{x.numerator}/{x.denominator}"
    digits, scale = 0, 1
    while (x * scale).denominator != 1:
        scale *= 10
        digits += 1
    n = int(x * scale)
    sign = "-" if n < 0 else ""
    s = str(abs(n)).rjust(digits + 1, "0")
    return f"{sign}{s[:-digits]}.{s[-digits:]}"


def paren(x) -> str:
    """Format with parentheses around negative numbers."""
    s = fmt(x)
    return f"({s})" if s.startswith("-") else s


def direct_product(a: Fraction, b: Fraction) -> Fraction:
    """Product of a and b by numerators and denominators, not via any reference."""
    return Fraction(a.numerator * b.numerator, a.denominator * b.denominator)


# ---------------------------------------------------------------------------
# Safe expression evaluation
# ---------------------------------------------------------------------------

_BINARY = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv,
}


def safe_eval(expr: str) -> Fraction:
    """Evaluate an arithmetic expression exactly, with no names, calls or attributes."""
    tree = ast.parse(expr.strip().replace("^", "**"), mode="eval")

    def ev(node) -> Fraction:
        if isinstance(node, ast.Expression):
            return ev(node.body)
        if isinstance(node, ast.Constant):
            if isinstance(node.value, (int, float)) and not isinstance(node.value, bool):
                return to_fraction(repr(node.value))
            raise ValueError("only numeric constants are allowed")
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.USub, ast.UAdd)):
            val = ev(node.operand)
            return -val if isinstance(node.op, ast.USub) else val
        if isinstance(node, ast.BinOp):
            op = type(node.op)
            if op in _BINARY:
                left, right = ev(node.left), ev(node.right)
                if op is ast.Div and right == 0:
                    raise ValueError("division by zero")
                return _BINARY[op](left, right)
            if op is ast.Pow:
                base, power = ev(node.left), ev(node.right)
                if power.denominator != 1 or abs(power) > 64:
                    raise ValueError("exponent must be an integer of magnitude at most 64")
                if base == 0 and power < 0:
                    raise ValueError("zero to a negative power")
                return base ** int(power)
        raise ValueError("unsupported syntax in expression")

    return ev(tree)


# ---------------------------------------------------------------------------
# Verified traces
# ---------------------------------------------------------------------------

@dataclass
class Step:
    claim: str
    lhs: Fraction
    rhs: Fraction

    @property
    def ok(self) -> bool:
        return self.lhs == self.rhs

    def render(self) -> str:
        mark = "ok  " if self.ok else "FAIL"
        return f"  [{mark}] {self.claim}   ({fmt(self.lhs)} = {fmt(self.rhs)})"


@dataclass
class Trace:
    a: Fraction
    b: Fraction
    m: Fraction
    u: Fraction
    v: Fraction
    terms: dict
    steps: list = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return all(s.ok for s in self.steps)

    @property
    def is_midpoint(self) -> bool:
        return self.m == (self.a + self.b) / 2

    def render(self) -> str:
        label = " (the midpoint)" if self.is_midpoint else ""
        lines = [
            f"Product {fmt(self.a)} x {fmt(self.b)} with reference m = {fmt(self.m)}{label}",
            f"  u = a - m = {fmt(self.u)}    v = b - m = {fmt(self.v)}",
            f"  m^2 = {fmt(self.terms['m^2'])}    m(u+v) = {fmt(self.terms['m(u+v)'])}"
            f"    uv = {fmt(self.terms['uv'])}",
        ]
        lines += [s.render() for s in self.steps]
        lines.append("  all equalities verified" if self.ok else "  AT LEAST ONE EQUALITY FAILED")
        return "\n".join(lines)


def build_trace(a, b, m=None) -> Trace:
    a, b = to_fraction(a), to_fraction(b)
    m = (a + b) / 2 if m is None else to_fraction(m)
    u, v = a - m, b - m
    terms = {"m^2": m * m, "m(u+v)": m * (u + v), "uv": u * v}
    total = terms["m^2"] + terms["m(u+v)"] + terms["uv"]
    trace = Trace(a, b, m, u, v, terms)
    trace.steps = [
        Step(f"a = m + u:  {fmt(m)} + {paren(u)}", m + u, a),
        Step(f"b = m + v:  {fmt(m)} + {paren(v)}", m + v, b),
        Step("(m+u)(m+v) = m^2 + m(u+v) + uv", (m + u) * (m + v), total),
        Step("m^2 + m(u+v) + uv = a*b, a*b computed directly", total, direct_product(a, b)),
    ]
    if u + v == 0:
        d = (a - b) / 2
        trace.steps.append(Step("u + v = 0, so the linear term m(u+v) vanishes", terms["m(u+v)"], Fraction(0)))
        trace.steps.append(Step("a*b = m^2 - d^2 with d = (a-b)/2", direct_product(a, b), m * m - d * d))
    return trace


# ---------------------------------------------------------------------------
# Checking a proposed transformation
# ---------------------------------------------------------------------------

@dataclass
class Verdict:
    a: Fraction
    b: Fraction
    m: Fraction
    claim: str
    claimed: Fraction
    actual: Fraction
    residual: Fraction
    hypotheses: list

    @property
    def correct(self) -> bool:
        return self.residual == 0

    def render(self) -> str:
        if self.correct:
            return (f"Claim {self.claim!r} equals {fmt(self.actual)}, the exact product "
                    f"{fmt(self.a)} x {fmt(self.b)}. Accepted.")
        lines = [
            f"Claim {self.claim!r} evaluates to {fmt(self.claimed)}, but "
            f"{fmt(self.a)} x {fmt(self.b)} = {fmt(self.actual)}.",
            f"Refused. Residual (claimed minus actual) = {fmt(self.residual)}.",
        ]
        if self.hypotheses:
            lines.append(f"Residual matches, relative to reference m = {fmt(self.m)}:")
            lines += [f"  - {h}" for h in self.hypotheses]
            lines.append("These are hypotheses about the slip. Other slips can produce the same residual.")
        else:
            lines.append("The residual matches no omitted or sign-flipped term for this reference.")
        return "\n".join(lines)


def diagnose(residual: Fraction, terms: dict, u: Optional[Fraction] = None,
             v: Optional[Fraction] = None) -> list:
    """Name slips whose effect equals the residual.

    Three classes are tried. If a subset S of the terms is left out,
    claimed = actual - sum(S), so the residual is -sum(S). If the signs of S
    are flipped, the residual is -2 sum(S). If the product uv is replaced by a
    single factor (u, v, -u or -v), as when a correction is written down but
    not squared or multiplied, the residual is that factor minus uv.
    Terms equal to zero cannot be meaningfully omitted or flipped, so they are
    left out of the candidates, and subsets whose sum is zero are skipped.
    """
    if residual == 0:
        return []
    names = [n for n in terms if terms[n] != 0]
    found = []
    for size in range(1, len(names) + 1):
        for subset in itertools.combinations(names, size):
            total = sum(terms[n] for n in subset)
            if total == 0:
                continue
            label = " + ".join(subset)
            if residual == -total:
                found.append(f"omitted: {label} (= {fmt(total)})")
            if residual == -2 * total:
                found.append(f"sign flipped on: {label} (= {fmt(total)})")
    if u is not None and v is not None:
        by_value: dict = {}
        for name, val in (("u", u), ("v", v), ("-u", -u), ("-v", -v)):
            by_value.setdefault(val, []).append(name)
        for val, labels in by_value.items():
            if val != terms["uv"] and residual == val - terms["uv"]:
                found.append(
                    f"uv replaced by a single factor ({' or '.join(labels)}, = {fmt(val)}): "
                    "correction not multiplied or squared"
                )
    return found


def check_claim(a, b, claim: str, m=None) -> Verdict:
    a, b = to_fraction(a), to_fraction(b)
    m = (a + b) / 2 if m is None else to_fraction(m)
    claimed = safe_eval(claim)
    actual = direct_product(a, b)
    u, v = a - m, b - m
    terms = {"m^2": m * m, "m(u+v)": m * (u + v), "uv": u * v}
    residual = claimed - actual
    return Verdict(a, b, m, claim, claimed, actual, residual, diagnose(residual, terms, u, v))


def log_entry(path, verdict: Verdict, justification: str = "", now: Optional[datetime] = None) -> dict:
    """Append one JSON line recording the proposal and the reason given for it."""
    now = now or datetime.now(timezone.utc)
    record = {
        "time": now.isoformat(),
        "a": fmt(verdict.a),
        "b": fmt(verdict.b),
        "reference": fmt(verdict.m),
        "claim": verdict.claim,
        "justification": justification,
        "claimed": fmt(verdict.claimed),
        "actual": fmt(verdict.actual),
        "residual": fmt(verdict.residual),
        "correct": verdict.correct,
        "hypotheses": verdict.hypotheses,
    }
    with open(path, "a", encoding="utf-8") as fh:
        fh.write(json.dumps(record, ensure_ascii=False) + "\n")
    return record


# ---------------------------------------------------------------------------
# Exploring references against a stated repertoire
# ---------------------------------------------------------------------------

def is_round(x, extra: Iterable[int] = (), digit_max: int = 9) -> bool:
    """True for 0, or an integer of the form d * 10^k with 1 <= d <= digit_max.

    Numbers listed in `extra` also count. Non-integers never do.
    """
    x = to_fraction(x)
    if x.denominator != 1:
        return False
    n = abs(x.numerator)
    if n == 0 or n in set(extra):
        return True
    while n % 10 == 0:
        n //= 10
    return n <= digit_max


@dataclass
class Row:
    m: Fraction
    u: Fraction
    v: Fraction
    terms: dict
    in_repertoire: dict

    @property
    def all_in_repertoire(self) -> bool:
        return all(self.in_repertoire.values())


def explore(a, b, lo=None, hi=None, halves=False, extra: Iterable[int] = (), digit_max: int = 9) -> list:
    a, b = to_fraction(a), to_fraction(b)
    lo = math.floor(min(a, b)) - 10 if lo is None else to_fraction(lo)
    hi = math.ceil(max(a, b)) + 10 if hi is None else to_fraction(hi)
    step = Fraction(1, 2) if halves else Fraction(1)
    extra = tuple(extra)
    rows, m = [], to_fraction(lo)
    while m <= hi:
        u, v = a - m, b - m
        rnd = lambda x: is_round(x, extra, digit_max)  # noqa: E731
        flags = {
            "m^2": rnd(m),
            "m(u+v)": rnd(m) and rnd(u + v),
            "uv": rnd(u) and rnd(v),
        }
        rows.append(Row(m, u, v, {"m^2": m * m, "m(u+v)": m * (u + v), "uv": u * v}, flags))
        m += step
    return rows


def render_rows(rows: Sequence[Row], show_all: bool) -> str:
    shown = [r for r in rows if show_all or r.all_in_repertoire]
    if not shown:
        return "No reference in the range leaves every term inside the stated repertoire."
    lines = []
    for r in shown:
        flags = " ".join(f"{k}:{'yes' if ok else 'no'}" for k, ok in r.in_repertoire.items())
        lines.append(
            f"  m = {fmt(r.m):>6}  u = {fmt(r.u):>5}  v = {fmt(r.v):>5}   "
            f"m^2 = {fmt(r.terms['m^2'])}, m(u+v) = {fmt(r.terms['m(u+v)'])}, uv = {fmt(r.terms['uv'])}   [{flags}]"
        )
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = p.add_subparsers(dest="cmd", required=True)

    t = sub.add_parser("trace", help="print a verified decomposition")
    t.add_argument("a")
    t.add_argument("b")
    t.add_argument("--ref")

    c = sub.add_parser("check", help="test a proposed expression for the product")
    c.add_argument("a")
    c.add_argument("b")
    c.add_argument("--claim", required=True)
    c.add_argument("--ref")
    c.add_argument("--justification", default="")
    c.add_argument("--log")

    e = sub.add_parser("explore", help="list references whose terms stay in a stated repertoire")
    e.add_argument("a")
    e.add_argument("b")
    e.add_argument("--lo")
    e.add_argument("--hi")
    e.add_argument("--halves", action="store_true")
    e.add_argument("--extra-known", type=int, nargs="*", default=[])
    e.add_argument("--all", action="store_true")
    return p


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.cmd == "trace":
            trace = build_trace(args.a, args.b, args.ref)
            print(trace.render())
            return 0 if trace.ok else 1
        if args.cmd == "check":
            verdict = check_claim(args.a, args.b, args.claim, args.ref)
            print(verdict.render())
            if args.log:
                log_entry(args.log, verdict, args.justification)
            return 0 if verdict.correct else 1
        if args.cmd == "explore":
            rows = explore(args.a, args.b, args.lo, args.hi, args.halves, args.extra_known)
            print(f"Repertoire: round numbers (digit times a power of ten), plus {args.extra_known or 'nothing else'}.")
            print(render_rows(rows, args.all))
            return 0
    except (ValueError, ZeroDivisionError, SyntaxError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 2


if __name__ == "__main__":
    sys.exit(main())
