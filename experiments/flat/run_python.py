#!/usr/bin/env python3
"""Independent Python oracle for the flat fixture suite.

This is a from-scratch reimplementation of the four-primitive kernel
(Pop, Refuse, Bind, Collapse) described in `Spherepop_Specifications.tex`
and executed as Rust in `spherepop-kernel/src/{event,history,arbiter,
collapse,sugar}.rs`. It deliberately does not import or call into the
Rust crate: agreement between this file and the Rust fixture runner
(`spherepop-kernel/src/bin/fixtures.rs`) on every fixture in
`experiments/flat/fixtures/` is itself a (small, two-implementation)
cross-implementation conformance result, per the tracking issue's
"Portable" evidence criterion.

Usage:
    python3 run_python.py [path/to/fixtures/dir]

Exits non-zero if any fixture fails.
"""
from __future__ import annotations

import json
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


WIRE_MAGIC = b"SPHIST1\0"


def _wire_string(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return struct.pack(">I", len(encoded)) + encoded


def encode_history(omega0, rules, history: list[Event]) -> bytes:
    """Encode a replayable world using the normative SPHIST/1 envelope."""
    out = bytearray(WIRE_MAGIC)
    omega_values = list(omega0)
    rule_values = list(rules)
    if len(omega_values) != len(set(omega_values)):
        raise ValueError("duplicate initial option")
    if len(rule_values) != len(set(rule_values)):
        raise ValueError("duplicate certified rule")
    omega = sorted(omega_values)
    certified = sorted(rule_values)
    out += struct.pack(">I", len(omega))
    for object_id in omega:
        out += struct.pack(">Q", object_id)
    out += struct.pack(">I", len(certified))
    for rule_name in certified:
        out += _wire_string(rule_name)
    out += struct.pack(">I", len(history))
    for event in history:
        kinds = {"pop": 0, "refuse": 1, "bind": 2, "collapse": 3}
        out.append(kinds[event.kind])
        if event.kind == "pop":
            out += struct.pack(">Q", event.a)
        elif event.kind == "refuse":
            out += struct.pack(">Q", event.a)
            out.append(1 if event.b is not None else 0)
            if event.b is not None:
                out += struct.pack(">Q", event.b)
            out += _wire_string(event.reason or "")
        elif event.kind == "bind":
            out += struct.pack(">QQ", event.a, event.b)
            out += _wire_string(event.tag or "")
        else:
            out += _wire_string(event.rule or "")
    return bytes(out)


def decode_history(data: bytes) -> tuple[list[int], list[str], list[Event]]:
    """Decode SPHIST/1 and reject truncation, invalid UTF-8, or trailing bytes."""
    view = memoryview(data)
    offset = 0

    def take(n: int) -> bytes:
        nonlocal offset
        if n < 0 or offset + n > len(view):
            raise ValueError("truncated SPHIST/1 envelope")
        value = bytes(view[offset:offset + n])
        offset += n
        return value

    def u32() -> int:
        return struct.unpack(">I", take(4))[0]

    def u64() -> int:
        return struct.unpack(">Q", take(8))[0]

    def string() -> str:
        return take(u32()).decode("utf-8")

    if take(len(WIRE_MAGIC)) != WIRE_MAGIC:
        raise ValueError("invalid SPHIST/1 magic")
    omega = [u64() for _ in range(u32())]
    rules = [string() for _ in range(u32())]
    history = []
    for _ in range(u32()):
        kind = take(1)[0]
        if kind == 0:
            history.append(pop(u64()))
        elif kind == 1:
            a = u64()
            has_b = take(1)[0]
            if has_b not in (0, 1):
                raise ValueError("invalid Refuse has_b flag")
            b = u64() if has_b else None
            history.append(refuse(a, string(), b=b))
        elif kind == 2:
            history.append(bind(u64(), u64(), string()))
        elif kind == 3:
            history.append(collapse(string()))
        else:
            raise ValueError(f"invalid event kind {kind}")
    if offset != len(view):
        raise ValueError("trailing bytes in SPHIST/1 envelope")
    if omega != sorted(set(omega)) or rules != sorted(set(rules)):
        raise ValueError("non-canonical SPHIST/1 set ordering")
    return omega, rules, history


def fnv1a64(data: bytes) -> str:
    digest = 0xCBF29CE484222325
    for byte in data:
        digest ^= byte
        digest = (digest * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return f"{digest:016x}"


# ---------------------------------------------------------------------
# Kernel: events, state, replay (mirrors spherepop-kernel/src/event.rs
# and history.rs).
# ---------------------------------------------------------------------

@dataclass(frozen=True)
class Event:
    kind: str  # "pop" | "refuse" | "bind" | "collapse"
    a: Optional[int] = None
    b: Optional[int] = None
    tag: Optional[str] = None
    reason: Optional[str] = None
    rule: Optional[str] = None


def pop(x: int) -> Event:
    return Event(kind="pop", a=x)


def refuse(x: Optional[int], reason: str, b: Optional[int] = None) -> Event:
    return Event(kind="refuse", a=x, b=b, reason=reason)


def bind(a: int, b: int, tag: str) -> Event:
    return Event(kind="bind", a=a, b=b, tag=tag)


def collapse(rule: str) -> Event:
    return Event(kind="collapse", rule=rule)


@dataclass
class State:
    option_space: set = field(default_factory=set)
    committed: set = field(default_factory=set)
    bound: set = field(default_factory=set)  # {(a, b, tag)}
    refused: list = field(default_factory=list)  # [(pos, a, reason)]
    observed: list = field(default_factory=list)  # [(pos, rule)]


def apply(state: State, event: Event) -> None:
    """The pure per-event transition function (apply :: State -> Event ->
    State in the Rust kernel). Deliberately has no side channel other
    than `state`, matching Deterministic Replay."""
    if event.kind == "pop":
        state.option_space.discard(event.a)
        state.committed.add(event.a)
    elif event.kind == "refuse":
        state.refused.append((len(state.observed) + len(state.refused), event.a, event.reason or ""))
        # Omega untouched: refusal documents, it does not foreclose.
    elif event.kind == "bind":
        state.bound.add((event.a, event.b, event.tag or ""))
    elif event.kind == "collapse":
        state.observed.append((len(state.observed) + len(state.refused), event.rule))
    else:
        raise ValueError(f"unknown event kind {event.kind!r}")


class ArbiterError(Exception):
    pass


class Arbiter:
    """The only path by which history is ever extended (mirrors
    spherepop-kernel/src/arbiter.rs)."""

    def __init__(self, omega0, rules):
        self.omega0 = set(omega0)
        self.rules = set(rules)
        self.history: list[Event] = []

    def state(self) -> State:
        s = State(option_space=set(self.omega0))
        for e in self.history:
            apply(s, e)
        return s

    def submit(self, events: list[Event]) -> None:
        s = self.state()
        hypothetical_committed = set()
        for e in events:
            if e.kind == "pop":
                if e.a not in s.option_space or e.a in hypothetical_committed:
                    raise ArbiterError(f"PopOutsideOptionSpace({e.a})")
                hypothetical_committed.add(e.a)
            elif e.kind == "refuse":
                if not (e.reason or ""):
                    raise ArbiterError("RefuseWithoutReason")
            elif e.kind == "bind":
                if e.a is None or e.b is None:
                    raise ArbiterError("Malformed(Bind missing a/b)")
            elif e.kind == "collapse":
                if e.rule is None:
                    raise ArbiterError("Malformed(Collapse missing rule)")
                if e.rule not in self.rules:
                    raise ArbiterError(f"UncertifiedCollapseRule({e.rule!r})")
            else:
                raise ArbiterError(f"Malformed(unknown kind {e.kind!r})")
        # All-or-nothing: only mutate history after every event validates.
        self.history.extend(events)


# ---------------------------------------------------------------------
# Collapse rules (mirrors spherepop-kernel/src/collapse.rs).
# ---------------------------------------------------------------------

class UnionFind:
    def __init__(self):
        self.parent = {}

    def root(self, x):
        p = self.parent.setdefault(x, x)
        if p == x:
            return x
        r = self.root(p)
        self.parent[x] = r
        return r

    def union(self, a, b):
        ra, rb = self.root(a), self.root(b)
        if ra != rb:
            self.parent[ra] = rb

    def same_class(self, a, b):
        return self.root(a) == self.root(b)


def collapse_quotient(history: list[Event]) -> UnionFind:
    uf = UnionFind()
    for e in history:
        if e.kind == "bind" and e.tag != "__meta__":
            uf.union(e.a, e.b)
    return uf


def collapse_quotient_honoring_refusals(history: list[Event]) -> UnionFind:
    uf = UnionFind()
    withdrawn = {
        (e.a, e.b) for e in history if e.kind == "refuse" and e.reason == "relation withdrawn"
    }
    for e in history:
        if e.kind == "bind" and e.tag != "__meta__":
            if (e.a, e.b) not in withdrawn and (e.b, e.a) not in withdrawn:
                uf.union(e.a, e.b)
    return uf


def collapse_meta(history: list[Event]) -> dict:
    out: dict = {}
    for e in history:
        if e.kind == "bind" and e.tag == "__meta__":
            out.setdefault(e.a, []).append(e.b)
    return out


# ---------------------------------------------------------------------
# Surface-calculus sugar (mirrors spherepop-kernel/src/sugar.rs). Every
# function here returns only primitive events.
# ---------------------------------------------------------------------

def link(a: int, b: int, tag: str) -> list[Event]:
    return [bind(a, b, tag)]


def unlink(a: int, b: int) -> list[Event]:
    return [refuse(a, "relation withdrawn", b=b)]


def choice(taken: int, rejected: int) -> list[Event]:
    return [pop(taken), refuse(rejected, "not selected by Choice")]


def merge(a: int, b: int, rule: str) -> list[Event]:
    return [bind(a, b, "merge"), collapse(rule)]


def set_meta(obj: int, key: int) -> list[Event]:
    return [bind(obj, key, "__meta__")]


def events_from_op(op: dict) -> list[Event]:
    kind = op["op"]
    if kind == "pop":
        return [pop(op["a"])]
    if kind == "refuse":
        return [refuse(op["a"], op.get("reason", ""))]
    if kind == "refuse_bind":
        return [refuse(op["a"], op.get("reason", ""), b=op["b"])]
    if kind == "bind":
        return [bind(op["a"], op["b"], op.get("tag", ""))]
    if kind == "collapse":
        return [collapse(op["rule"])]
    if kind == "link":
        return link(op["a"], op["b"], op.get("tag", ""))
    if kind == "unlink":
        return unlink(op["a"], op["b"])
    if kind == "choice":
        return choice(op["taken"], op["rejected"])
    if kind == "merge":
        return merge(op["a"], op["b"], op["rule"])
    if kind == "set_meta":
        return set_meta(op["object"], op["key"])
    raise ValueError(f"unknown fixture op {kind!r}")


# ---------------------------------------------------------------------
# Fixture runner.
# ---------------------------------------------------------------------

def run_meld_fixture(fixture: dict) -> list[str]:
    """Executes a two-history Meld fixture (`history_a`/`history_b`) end-to-end:
    each sub-history is submitted through its own Arbiter, then the two
    resulting histories are melded (event-log concatenation, matching
    `spherepop-kernel::History::meld`) and checked against
    `expect_melded_history_len`."""
    failures: list[str] = []
    histories = []
    for key in ("history_a", "history_b"):
        sub = fixture[key]
        arb = Arbiter(set(sub["initial_option_space"]), set(sub.get("certified_rules", [])))
        for ev in sub.get("events", []):
            arb.submit(events_from_op(ev))
        histories.append(arb.history)

    melded = histories[0] + histories[1]
    expected_len = fixture.get("expect_melded_history_len")
    if expected_len is not None and len(melded) != expected_len:
        failures.append(f"expect_melded_history_len: expected {expected_len}, got {len(melded)}")
    return failures


def run_fixture(path: Path) -> list[str]:
    """Returns a list of failure messages; empty means the fixture passed."""
    fixture = json.loads(path.read_text())
    failures: list[str] = []

    if "history_a" in fixture:
        return run_meld_fixture(fixture)

    if fixture.get("manual"):
        for required in ("invariant", "explanation"):
            if required not in fixture:
                failures.append(f"manual fixture missing {required!r}")
        return failures

    omega0 = set(fixture["initial_option_space"])
    rules = set(fixture.get("certified_rules", []))
    arb = Arbiter(omega0, rules)

    for ev in fixture.get("events", []):
        events = events_from_op(ev)
        expect_reject = ev.get("expect_reject", False)
        len_before = len(arb.history)
        try:
            arb.submit(events)
            if expect_reject:
                failures.append(f"event {ev!r} was accepted but fixture expected rejection")
        except ArbiterError as e:
            if not expect_reject:
                failures.append(f"event {ev!r} was rejected unexpectedly: {e}")
                continue
            if len(arb.history) != len_before:
                failures.append("rejected event mutated history length")
            expected_err = ev.get("expect_error")
            if expected_err and not str(e).startswith(expected_err):
                failures.append(f"expected error prefix {expected_err!r}, got {e}")

    state = arb.state()
    expect = fixture.get("expect", {})

    if "option_space" in expect:
        if state.option_space != set(expect["option_space"]):
            failures.append(f"option_space: expected {sorted(expect['option_space'])}, got {sorted(state.option_space)}")
    if "committed" in expect:
        if state.committed != set(expect["committed"]):
            failures.append(f"committed: expected {sorted(expect['committed'])}, got {sorted(state.committed)}")
    if "refused_count" in expect:
        if len(state.refused) != expect["refused_count"]:
            failures.append(f"refused_count: expected {expect['refused_count']}, got {len(state.refused)}")
    if "history_len" in expect:
        if len(arb.history) != expect["history_len"]:
            failures.append(f"history_len: expected {expect['history_len']}, got {len(arb.history)}")
    if "observed_rules" in expect:
        actual = [r for (_, r) in state.observed]
        if actual != expect["observed_rules"]:
            failures.append(f"observed_rules: expected {expect['observed_rules']}, got {actual}")
    if "bound" in expect:
        for a, b, tag in expect["bound"]:
            if (a, b, tag) not in state.bound:
                failures.append(f"bound: expected ({a}, {b}, {tag!r}) to be present")
    if "quotient_same_class" in expect:
        uf = collapse_quotient(arb.history)
        for a, b, expected in expect["quotient_same_class"]:
            actual = uf.same_class(a, b)
            if actual != expected:
                failures.append(f"quotient_same_class({a}, {b}): expected {expected}, got {actual}")
    if "quotient_honoring_refusals_same_class" in expect:
        uf = collapse_quotient_honoring_refusals(arb.history)
        for a, b, expected in expect["quotient_honoring_refusals_same_class"]:
            actual = uf.same_class(a, b)
            if actual != expected:
                failures.append(f"quotient_honoring_refusals_same_class({a}, {b}): expected {expected}, got {actual}")
    if "meta_keys" in expect:
        meta = collapse_meta(arb.history)
        for key in expect["meta_keys"]:
            if key not in meta:
                failures.append(f"meta_keys: expected object {key} to have metadata")
    if expect.get("deterministic_replay"):
        s1 = arb.state()
        s2 = arb.state()
        if s1 != s2:
            failures.append("deterministic_replay: two replays of the same history disagreed")
    if "canonical_history_fnv1a64" in expect:
        wire = encode_history(omega0, rules, arb.history)
        actual_digest = fnv1a64(wire)
        if actual_digest != expect["canonical_history_fnv1a64"]:
            failures.append(
                "canonical_history_fnv1a64: expected "
                f"{expect['canonical_history_fnv1a64']}, got {actual_digest}"
            )
        decoded_omega, decoded_rules, decoded_history = decode_history(wire)
        replayed = State(option_space=set(decoded_omega))
        for event in decoded_history:
            apply(replayed, event)
        if replayed != state:
            failures.append("wire_replay: decoded history produced a different state")
        if decoded_rules != sorted(rules):
            failures.append("wire_replay: certified rules changed during round trip")
        try:
            decode_history(wire + b"\0")
            failures.append("wire_decode: accepted trailing bytes")
        except ValueError:
            pass

    return failures


def main() -> int:
    default_dir = Path(__file__).resolve().parent / "fixtures"
    fixtures_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else default_dir

    paths = sorted(fixtures_dir.glob("*.json"))
    passed = failed = skipped = 0
    for path in paths:
        name = path.stem
        try:
            failures = run_fixture(path)
        except Exception as e:  # fixture-level crash counts as a failure
            failures = [f"runner exception: {e!r}"]
        fixture = json.loads(path.read_text())
        if fixture.get("manual") and not failures:
            print(f"SKIP  {name} (manual/structural only)")
            skipped += 1
            continue
        if failures:
            print(f"FAIL  {name}")
            for f in failures:
                print(f"      - {f}")
            failed += 1
        else:
            print(f"PASS  {name}")
            passed += 1

    print(f"\n{passed} passed, {failed} failed, {skipped} manual, {passed + failed + skipped} total")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
