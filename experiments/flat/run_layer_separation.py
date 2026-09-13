#!/usr/bin/env python3
"""Executable boundary check for Spherepop history, state, and rendering.

This is intentionally separate from the cross-implementation kernel fixture
matrix.  SPHIST/1 is the authoritative replay input; ``State`` is derived by
replay; renderers consume an immutable canonical state projection and are not
part of either the history envelope or the kernel semantics.

Usage:
    python3 run_layer_separation.py [path/to/layer_fixture.json]
"""
from __future__ import annotations

import copy
import json
import sys
from pathlib import Path

from run_python import (
    Arbiter,
    State,
    apply,
    decode_history,
    encode_history,
    events_from_op,
    fnv1a64,
)


def state_projection(state: State) -> dict:
    """Return the canonical data handed to renderers, without presentation."""
    return {
        "bound": [list(item) for item in sorted(state.bound)],
        "committed": sorted(state.committed),
        "observed": [list(item) for item in state.observed],
        "option_space": sorted(state.option_space),
        "refused": [list(item) for item in state.refused],
    }


def render_compact(view: dict) -> str:
    """One deliberately opinionated human-readable renderer."""
    bound = ",".join(f"{a}-{tag}->{b}" for a, b, tag in view["bound"])
    committed = ",".join(map(str, view["committed"]))
    observed = ",".join(rule for _, rule in view["observed"])
    omega = ",".join(map(str, view["option_space"]))
    refused = ",".join(f"{a}:{reason}" for _, a, reason in view["refused"])
    return (
        f"Omega[{omega}] | committed[{committed}] | bound[{bound}] | "
        f"refused[{refused}] | observed[{observed}]"
    )


def render_json(view: dict) -> str:
    """A machine-oriented renderer of the same semantic projection."""
    return json.dumps(view, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def run(path: Path) -> dict:
    fixture = json.loads(path.read_text())
    omega0 = set(fixture["initial_option_space"])
    rules = set(fixture.get("certified_rules", []))
    arbiter = Arbiter(omega0, rules)
    for op in fixture["events"]:
        arbiter.submit(events_from_op(op))

    wire = encode_history(omega0, rules, arbiter.history)
    digest_before = fnv1a64(wire)
    decoded_omega, decoded_rules, decoded_events = decode_history(wire)

    replayed = State(option_space=set(decoded_omega))
    for event in decoded_events:
        apply(replayed, event)
    view = state_projection(replayed)
    view_before_render = copy.deepcopy(view)

    compact = render_compact(view)
    machine = render_json(view)

    # Re-encode from decoded replay inputs after both renderers have run.  A
    # renderer with a hidden mutation or history dependency breaks this check.
    wire_after = encode_history(decoded_omega, decoded_rules, decoded_events)
    digest_after = fnv1a64(wire_after)
    expect = fixture["expect"]

    checks = {
        "history_round_trip": wire_after == wire,
        "history_unchanged_by_rendering": digest_after == digest_before,
        "history_digest": digest_before == expect["history_fnv1a64"],
        "state_is_replay_derived": replayed == arbiter.state(),
        "state_unchanged_by_rendering": view == view_before_render,
        "state_projection": view == expect["state"],
        "compact_rendering": compact == expect["compact_rendering"],
        "renderers_are_distinct": compact != machine,
        "renderers_are_deterministic": (
            compact == render_compact(view) and machine == render_json(view)
        ),
    }
    failed = [name for name, passed in checks.items() if not passed]
    if failed:
        raise AssertionError("failed checks: " + ", ".join(failed))

    return {
        "history": {"event_count": len(decoded_events), "fnv1a64": digest_before},
        "state": view,
        "renderings": {"compact": compact, "json": machine},
        "checks": checks,
    }


def main() -> int:
    default = Path(__file__).resolve().parent / "layer_fixtures" / "01_history_state_rendering.json"
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else default
    try:
        result = run(path)
    except Exception as exc:
        print(f"FAIL  {path.stem}: {exc}")
        return 1
    print(f"PASS  {path.stem}")
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
