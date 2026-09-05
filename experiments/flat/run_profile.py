#!/usr/bin/env python3
"""Runner for the non-authoritative `profile.py` fixtures in
`profile_fixtures/`, per `COMPLEXITY.md` section 14.

Kept entirely separate from `run_python.py` and its `fixtures/` directory
on purpose: these fixtures exercise derived analysis functions, not
kernel admissibility/replay, and must not be able to perturb the existing
conformance suite in any way.

Usage:
    python3 run_profile.py [path/to/profile_fixtures/dir]

Exits non-zero if any fixture fails.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import profile as pf  # noqa: E402
from run_python import Arbiter, events_from_op  # noqa: E402


def check_13(fixture: dict) -> list[str]:
    failures = []
    arb = Arbiter(set(fixture["initial_option_space"]), set(fixture.get("certified_rules", [])))
    expected = fixture["expect_lengths_after_each_op"]
    for i, op in enumerate(fixture["ops"]):
        arb.submit(events_from_op(op))
        got = pf.event_length(arb.history)
        if got != expected[i]:
            failures.append(f"op {i} ({op}): expected length {expected[i]}, got {got}")
    return failures


def check_14(fixture: dict) -> list[str]:
    failures = []
    omega0 = set(fixture["common_prefix_option_space"])
    rules = set(fixture.get("certified_rules", []))

    results = {}
    for branch in ("branch_a_ops", "branch_b_ops"):
        arb = Arbiter(set(omega0), set(rules))
        for op in fixture["common_prefix_ops"]:
            arb.submit(events_from_op(op))
        prefix_len = len(arb.history)
        state_before = arb.state()
        for op in fixture[branch]:
            arb.submit(events_from_op(op))
        results[branch] = (prefix_len, state_before)

    (len_a, state_a), (len_b, state_b) = results["branch_a_ops"], results["branch_b_ops"]
    if len_a != len_b or len_a != fixture["expect_prefix_history_len"]:
        failures.append(f"prefix history_len mismatch: a={len_a}, b={len_b}, expected={fixture['expect_prefix_history_len']}")
    if state_a.option_space != set(fixture["expect_prefix_option_space"]) or state_b.option_space != set(fixture["expect_prefix_option_space"]):
        failures.append("prefix option_space differs between branches or from expectation")
    if state_a.committed != set(fixture["expect_prefix_committed"]) or state_b.committed != set(fixture["expect_prefix_committed"]):
        failures.append("prefix committed set differs between branches or from expectation")
    return failures


def check_15(fixture: dict) -> list[str]:
    failures = []
    for key in ("history_1", "history_2"):
        sub = fixture[key]
        h = pf.build_history(sub["initial_option_space"], sub.get("certified_rules", []), sub["ops"])
        exp = fixture["expect"][key]
        got = {
            "num_components": pf.num_components(h),
            "depth": pf.depth(h),
            "width": pf.width(h),
        }
        for k, v in exp.items():
            if got[k] != v:
                failures.append(f"{key}.{k}: expected {v}, got {got[k]}")
    return failures


def check_16(fixture: dict) -> list[str]:
    failures = []
    for key in ("history_shared", "history_no_reuse"):
        sub = fixture[key]
        h = pf.build_history(sub["initial_option_space"], sub.get("certified_rules", []), sub["ops"])
        tree = pf.tree_cost(h)
        dag = pf.dag_cost(h)
        gain = pf.reuse_gain(h)
        exp = fixture["expect"][key]
        if abs(tree - exp["tree_cost"]) > 1e-9:
            failures.append(f"{key}.tree_cost: expected {exp['tree_cost']}, got {tree}")
        if abs(dag - exp["dag_cost"]) > 1e-9:
            failures.append(f"{key}.dag_cost: expected {exp['dag_cost']}, got {dag}")
        if (gain > 0) != exp["reuse_gain_positive"]:
            failures.append(f"{key}.reuse_gain_positive: expected {exp['reuse_gain_positive']}, got gain={gain}")
    return failures


def check_17(fixture: dict) -> list[str]:
    failures = []
    rules = set(fixture.get("certified_rules", []))
    target = tuple(fixture["target"])
    histories = {}
    for key in ("candidate_coarse", "candidate_fine"):
        sub = fixture[key]
        h = pf.build_history(sub["initial_option_space"], rules, sub["ops"])
        histories[key] = h
        exp = fixture["expect"][key]
        length = pf.event_length(h)
        if length != exp["length"]:
            failures.append(f"{key}.length: expected {exp['length']}, got {length}")
        for rule_name in ("coarse", "fine"):
            got = pf.observes_same_class(rule_name, h, *target)
            expected = exp[f"same_class_under_{rule_name}"]
            if got != expected:
                failures.append(f"{key}.same_class_under_{rule_name}: expected {expected}, got {got}")

    candidates = list(histories.values())
    min_coarse = pf.min_length_achieving("coarse", candidates, target)
    min_fine = pf.min_length_achieving("fine", candidates, target)
    if min_coarse != fixture["expect"]["min_length_achieving_coarse"]:
        failures.append(f"min_length_achieving(coarse): expected {fixture['expect']['min_length_achieving_coarse']}, got {min_coarse}")
    if min_fine != fixture["expect"]["min_length_achieving_fine"]:
        failures.append(f"min_length_achieving(fine): expected {fixture['expect']['min_length_achieving_fine']}, got {min_fine}")
    strictly_greater = (min_fine is not None and min_coarse is not None and min_fine > min_coarse)
    if strictly_greater != fixture["expect"]["strictly_greater"]:
        failures.append(f"strictly_greater: expected {fixture['expect']['strictly_greater']}, got {strictly_greater}")
    return failures


CHECKS = {
    "13_length_increases.json": check_13,
    "14_collapse_non_mutating.json": check_14,
    "15_depth_vs_width.json": check_15,
    "16_reuse_tree_vs_dag.json": check_16,
    "17_collapse_relative_minimal_assembly.json": check_17,
}


def main() -> int:
    default_dir = Path(__file__).resolve().parent / "profile_fixtures"
    fixtures_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else default_dir

    passed = failed = 0
    for name, check in CHECKS.items():
        path = fixtures_dir / name
        if not path.exists():
            print(f"MISSING {name}")
            failed += 1
            continue
        fixture = json.loads(path.read_text())
        try:
            failures = check(fixture)
        except Exception as e:
            failures = [f"runner exception: {e!r}"]
        if failures:
            print(f"FAIL  {name}")
            for f in failures:
                print(f"      - {f}")
            failed += 1
        else:
            print(f"PASS  {name}")
            passed += 1

    print(f"\n{passed} passed, {failed} failed, {passed + failed} total")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
