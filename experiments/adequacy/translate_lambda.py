#!/usr/bin/env python3
"""Phase E adequacy experiment: translate a tiny lambda-calculus fragment
into Spherepop primitive-event traces and check preservation of the
observed result against an independent reference evaluator.

See lambda_calculus.md for the full definition of the source calculus,
the translation, and the read-back ("observation") relation this file
implements and checks. This file deliberately imports and reuses the
exact kernel (`Event`, `apply`, `Arbiter`, `ArbiterError`, `choice`) from
`experiments/flat/run_python.py` rather than reimplementing it, so every
translated trace is checked against the same validated kernel the flat
fixture suite already exercises.

Usage:
    python3 translate_lambda.py [--write-traces]

Exits non-zero if any program's translated trace is rejected by the
Arbiter, or if the reference-evaluator result disagrees with the
Spherepop-observed result.
"""
from __future__ import annotations

import json
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Union

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "flat"))
import run_python as rp  # noqa: E402  (the shared kernel; see module docstring)


# ---------------------------------------------------------------------
# 1. Source calculus: AST and reference (ground-truth) evaluator.
# ---------------------------------------------------------------------

@dataclass(frozen=True)
class Lit:
    n: int


@dataclass(frozen=True)
class Var:
    name: str


@dataclass(frozen=True)
class Lam:
    param: str
    body: object


@dataclass(frozen=True)
class App:
    fn: object
    arg: object


@dataclass(frozen=True)
class Add:
    e1: object
    e2: object


@dataclass(frozen=True)
class Sub:
    e1: object
    e2: object


@dataclass(frozen=True)
class If0:
    c: object
    t: object
    e: object


@dataclass(frozen=True)
class Let:
    name: str
    e1: object
    e2: object


@dataclass(frozen=True)
class Rec:
    name: str
    param: str
    body: object


@dataclass
class Closure:
    param: str
    body: object
    env: dict
    self_name: Optional[str] = None


Value = Union[int, Closure]


def pretty(e) -> str:
    if isinstance(e, Lit):
        return str(e.n)
    if isinstance(e, Var):
        return e.name
    if isinstance(e, Lam):
        return f"(\u03bb{e.param}. {pretty(e.body)})"
    if isinstance(e, App):
        return f"({pretty(e.fn)} {pretty(e.arg)})"
    if isinstance(e, Add):
        return f"({pretty(e.e1)} + {pretty(e.e2)})"
    if isinstance(e, Sub):
        return f"({pretty(e.e1)} - {pretty(e.e2)})"
    if isinstance(e, If0):
        return f"(if0 {pretty(e.c)} then {pretty(e.t)} else {pretty(e.e)})"
    if isinstance(e, Let):
        return f"(let {e.name} = {pretty(e.e1)} in {pretty(e.e2)})"
    if isinstance(e, Rec):
        return f"(rec {e.name}({e.param}) = {pretty(e.body)})"
    raise TypeError(f"unknown term {e!r}")


def eval_term(e, env: dict) -> Value:
    """Big-step reference evaluator: the independent ground truth this
    experiment's translation is checked against. Entirely unaware of
    Spherepop; a plain interpreter over `Value = int | Closure`."""
    if isinstance(e, Lit):
        return e.n
    if isinstance(e, Var):
        return env[e.name]
    if isinstance(e, Lam):
        return Closure(e.param, e.body, env)
    if isinstance(e, Rec):
        return Closure(e.param, e.body, env, self_name=e.name)
    if isinstance(e, App):
        fv = eval_term(e.fn, env)
        av = eval_term(e.arg, env)
        if not isinstance(fv, Closure):
            raise TypeError(f"applying a non-function: {fv!r}")
        new_env = dict(fv.env)
        if fv.self_name:
            new_env[fv.self_name] = fv
        new_env[fv.param] = av
        return eval_term(fv.body, new_env)
    if isinstance(e, Add):
        return eval_term(e.e1, env) + eval_term(e.e2, env)
    if isinstance(e, Sub):
        return eval_term(e.e1, env) - eval_term(e.e2, env)
    if isinstance(e, If0):
        cv = eval_term(e.c, env)
        return eval_term(e.t, env) if cv == 0 else eval_term(e.e, env)
    if isinstance(e, Let):
        return eval_term(e.e2, {**env, e.name: eval_term(e.e1, env)})
    raise TypeError(f"unknown term {e!r}")


# ---------------------------------------------------------------------
# 2. Translation: term -> Spherepop primitive-event trace.
#
# `TEnv` maps variable names to either a translated result ObjectId
# (int) or a `Closure` (translator-level only; closures are never
# Spherepop objects in their own right, only the integers their calls
# eventually commit to are).
# ---------------------------------------------------------------------

RESULT_RULE = "result"


def make_counter(start: int):
    box = [start]

    def fresh() -> int:
        box[0] += 1
        return box[0]

    return fresh


class Recorder:
    """Sizing-pass recorder: shares the exact `apply` function the real
    kernel uses, so `state.bound`/`state.committed` are identical to
    what a real Arbiter would compute, but performs no validation. Used
    only to discover the exact set of objects a program will Pop, so the
    real Arbiter below can be constructed with a tight, exact Omega_0
    rather than an oversized guess."""

    def __init__(self):
        self.history: list[rp.Event] = []
        self._state = rp.State()

    def submit(self, events: list[rp.Event]) -> None:
        for e in events:
            rp.apply(self._state, e)
            self.history.append(e)

    def state(self) -> rp.State:
        return self._state


def denote(events: list[rp.Event], obj: int) -> int:
    """collapse_result: a pure function of the event list alone, in the
    same style as `collapse_meta` in run_python.py -- reads back the
    literal integer a result object was bound to via the reserved
    "denotes" tag. This is the read-back relation defined in
    lambda_calculus.md Sec 2."""
    for e in events:
        if e.kind == "bind" and e.a == obj and e.tag == "denotes":
            return e.b
    raise LookupError(f"object {obj} was never bound to a denoted value")


def translate(term, tenv: dict, arb, fresh) -> Union[int, Closure]:
    """Emits primitive events into `arb` (an Arbiter-shaped object:
    Recorder during the sizing pass, a real rp.Arbiter for the checked
    run) and returns either a Spherepop ObjectId (int) denoting an
    integer result, or a translator-level Closure value."""
    if isinstance(term, Lit):
        v = fresh()
        arb.submit([rp.pop(v)])
        arb.submit([rp.bind(v, term.n, "denotes")])
        return v

    if isinstance(term, Var):
        return tenv[term.name]

    if isinstance(term, Lam):
        return Closure(term.param, term.body, tenv)

    if isinstance(term, Rec):
        return Closure(term.param, term.body, tenv, self_name=term.name)

    if isinstance(term, App):
        fv = translate(term.fn, tenv, arb, fresh)
        av = translate(term.arg, tenv, arb, fresh)
        if not isinstance(fv, Closure):
            raise TypeError(f"applying a non-function: {fv!r}")
        if isinstance(av, int):
            # Audit-bind only concrete (already-committed) values; a
            # parameter bound directly to a function value has no
            # Spherepop-level audit entry, since functions are not
            # object-space citizens in this experiment (see
            # lambda_calculus.md Sec 2's scope note).
            site = fresh()
            arb.submit([rp.bind(site, av, f"env:{fv.param}")])
        new_tenv = dict(fv.env)
        if fv.self_name:
            new_tenv[fv.self_name] = fv
        new_tenv[fv.param] = av
        return translate(fv.body, new_tenv, arb, fresh)

    if isinstance(term, (Add, Sub)):
        v1 = translate(term.e1, tenv, arb, fresh)
        v2 = translate(term.e2, tenv, arb, fresh)
        n1 = denote(_events_of(arb), v1)
        n2 = denote(_events_of(arb), v2)
        tag = "operand_of_+" if isinstance(term, Add) else "operand_of_-"
        arb.submit([rp.bind(v1, v2, tag)])
        result = n1 + n2 if isinstance(term, Add) else n1 - n2
        v3 = fresh()
        arb.submit([rp.pop(v3)])
        arb.submit([rp.bind(v3, result, "denotes")])
        return v3

    if isinstance(term, If0):
        cv = translate(term.c, tenv, arb, fresh)
        cn = denote(_events_of(arb), cv)
        taken_marker = fresh()
        rejected_marker = fresh()
        if cn == 0:
            arb.submit(rp.choice(taken_marker, rejected_marker))
            return translate(term.t, tenv, arb, fresh)
        else:
            arb.submit(rp.choice(taken_marker, rejected_marker))
            return translate(term.e, tenv, arb, fresh)

    if isinstance(term, Let):
        v1 = translate(term.e1, tenv, arb, fresh)
        if isinstance(v1, int):
            site = fresh()
            arb.submit([rp.bind(site, v1, f"env:{term.name}")])
        return translate(term.e2, {**tenv, term.name: v1}, arb, fresh)

    raise TypeError(f"unknown term {term!r}")


def _events_of(arb) -> list[rp.Event]:
    if isinstance(arb, Recorder):
        return arb.history
    return arb.history  # rp.Arbiter also exposes `.history`


# ---------------------------------------------------------------------
# 3. Two-pass execution: size Omega_0 exactly, then run for real through
#    a validated Arbiter. See lambda_calculus.md Sec 4.
# ---------------------------------------------------------------------

@dataclass
class RunResult:
    name: str
    program_src: str
    reference_value: int
    observed_value: int
    initial_option_space: list
    certified_rules: list
    events: list  # list of {"op": ...} dicts, flat-fixture-style


def run_program(name: str, term) -> RunResult:
    reference_value = eval_term(term, {})
    if not isinstance(reference_value, int):
        raise TypeError(f"program {name!r} must evaluate to an integer, got {reference_value!r}")

    # Pass 1: sizing. Discover exactly which objects get Popped.
    recorder = Recorder()
    root_obj_dry = translate(term, {}, recorder, make_counter(0))
    assert isinstance(root_obj_dry, int), f"program {name!r} must translate to a result object"
    omega0 = sorted(recorder.state().committed)

    # Pass 2: the real, validated run.
    arb = rp.Arbiter(omega0=omega0, rules={RESULT_RULE})
    root_obj = translate(term, {}, arb, make_counter(0))
    assert root_obj == root_obj_dry, "sizing pass and real pass diverged"
    arb.submit([rp.collapse(RESULT_RULE)])

    observed_value = denote(arb.history, root_obj)

    events_json = []
    for e in arb.history:
        d = {"op": e.kind}
        if e.a is not None:
            d["a"] = e.a
        if e.b is not None:
            d["b"] = e.b
        if e.tag is not None:
            d["tag"] = e.tag
        if e.reason is not None:
            d["reason"] = e.reason
        if e.rule is not None:
            d["rule"] = e.rule
        events_json.append(d)

    return RunResult(
        name=name,
        program_src=pretty(term),
        reference_value=reference_value,
        observed_value=observed_value,
        initial_option_space=omega0,
        certified_rules=sorted(arb.rules),
        events=events_json,
    )


# ---------------------------------------------------------------------
# 4. Example programs (Phase E checklist: names/bindings, composition,
#    reusable abstraction, conditional choice/refusal, recursion).
# ---------------------------------------------------------------------

def build_programs() -> list[tuple[str, object]]:
    literal = Lit(42)

    arithmetic = Sub(Add(Lit(2), Lit(3)), Lit(1))  # (2 + 3) - 1 = 4

    identity_application = App(Lam("x", Var("x")), Lit(7))

    double = Lam("x", Add(Var("x"), Var("x")))
    inc = Lam("x", Add(Var("x"), Lit(1)))
    composition = Let("double", double,
                       Let("inc", inc,
                           App(Var("inc"), App(Var("double"), Lit(4)))))  # inc(double(4)) = 9

    conditional_true = If0(Sub(Lit(3), Lit(3)), Lit(100), Lit(200))   # condition is 0 -> 100
    conditional_false = If0(Lit(1), Lit(100), Lit(200))               # condition != 0 -> 200

    # rec sum(n) = if0 n then 0 else n + sum(n - 1); sum(4) = 10.
    recursion_sum = App(
        Rec("sum", "n", If0(Var("n"), Lit(0), Add(Var("n"), App(Var("sum"), Sub(Var("n"), Lit(1)))))),
        Lit(4),
    )

    return [
        ("literal", literal),
        ("arithmetic", arithmetic),
        ("identity_application", identity_application),
        ("composition", composition),
        ("conditional_true", conditional_true),
        ("conditional_false", conditional_false),
        ("recursion_sum", recursion_sum),
    ]


# ---------------------------------------------------------------------
# 5. Runner.
# ---------------------------------------------------------------------

def main() -> int:
    write_traces = "--write-traces" in sys.argv
    traces_dir = Path(__file__).resolve().parent / "traces"

    failures = []
    for name, term in build_programs():
        try:
            result = run_program(name, term)
        except rp.ArbiterError as exc:
            print(f"FAIL  {name}  (rejected by Arbiter: {exc})")
            failures.append(name)
            continue
        except Exception as exc:  # noqa: BLE001 - report and continue
            print(f"FAIL  {name}  (exception: {exc!r})")
            failures.append(name)
            continue

        match = result.reference_value == result.observed_value
        status = "PASS" if match else "FAIL"
        print(f"{status}  {name}  reference={result.reference_value} "
              f"observed={result.observed_value}  events={len(result.events)}")
        if not match:
            failures.append(name)

        if write_traces:
            traces_dir.mkdir(parents=True, exist_ok=True)
            out = {
                "name": result.name,
                "program": result.program_src,
                "reference_value": result.reference_value,
                "observed_value": result.observed_value,
                "match": match,
                "initial_option_space": result.initial_option_space,
                "certified_rules": result.certified_rules,
                "events": result.events,
            }
            (traces_dir / f"{name}.json").write_text(json.dumps(out, indent=2) + "\n")

    total = len(build_programs())
    print(f"\n{total - len(failures)} passed, {len(failures)} failed, {total} total")
    if failures:
        print("FAILED: " + ", ".join(failures))
        return 1
    print("Lambda-calculus adequacy experiment: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
