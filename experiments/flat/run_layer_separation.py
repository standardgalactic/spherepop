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

import json
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Tuple

from run_python import (
    State,
    apply,
    decode_history,
    fnv1a64,
)


@dataclass(frozen=True)
class RenderView:
    """Immutable, canonical renderer input."""

    bound: Tuple[Tuple[int, int, str], ...]
    committed: Tuple[int, ...]
    observed: Tuple[Tuple[int, str], ...]
    option_space: Tuple[int, ...]
    refused: Tuple[Tuple[int, int | None, str], ...]

    def json_value(self) -> dict:
        """Return a fresh JSON value; never expose mutable renderer input."""
        return {
            "bound": [list(item) for item in self.bound],
            "committed": list(self.committed),
            "observed": [list(item) for item in self.observed],
            "option_space": list(self.option_space),
            "refused": [list(item) for item in self.refused],
        }


def state_projection(state: State) -> RenderView:
    """Return the canonical immutable value handed to renderers."""
    return RenderView(
        bound=tuple(sorted(state.bound)),
        committed=tuple(sorted(state.committed)),
        observed=tuple(state.observed),
        option_space=tuple(sorted(state.option_space)),
        refused=tuple(state.refused),
    )


def render_compact(view: RenderView) -> str:
    """One deliberately opinionated human-readable renderer."""
    bound = ",".join(f"{a}-{tag}->{b}" for a, b, tag in view.bound)
    committed = ",".join(map(str, view.committed))
    observed = ",".join(rule for _, rule in view.observed)
    omega = ",".join(map(str, view.option_space))
    refused = ",".join(f"{a}:{reason}" for _, a, reason in view.refused)
    return (
        f"Omega[{omega}] | committed[{committed}] | bound[{bound}] | "
        f"refused[{refused}] | observed[{observed}]"
    )


def render_json(view: RenderView) -> str:
    """A machine-oriented renderer of the same semantic projection."""
    return json.dumps(
        view.json_value(), ensure_ascii=False, sort_keys=True, separators=(",", ":")
    )


def run(path: Path) -> dict:
    fixture = json.loads(path.read_text())
    # The fixture supplies the authoritative replay input. Do not construct it
    # through Arbiter or encode_history here: that would make the producer and
    # consumer the same oracle and weaken the boundary check.
    wire = bytes.fromhex(fixture["history_sphist1_hex"])
    digest_before = fnv1a64(wire)
    decoded_omega, decoded_rules, decoded_events = decode_history(wire)

    replayed = State(option_space=set(decoded_omega))
    for event in decoded_events:
        apply(replayed, event)
    view = state_projection(replayed)
    state_before_render = state_projection(replayed)

    compact = render_compact(view)
    compact_is_deterministic = compact == render_compact(view)
    compact_preserves_state = state_projection(replayed) == state_before_render
    machine = render_json(view)
    machine_is_deterministic = machine == render_json(view)
    machine_preserves_state = state_projection(replayed) == state_before_render

    expect = fixture["expect"]

    checks = {
        "history_is_authoritative_fixture_input": bool(wire),
        "history_unchanged_by_rendering": fnv1a64(wire) == digest_before,
        "history_digest": digest_before == expect["history_fnv1a64"],
        "state_unchanged_by_compact_rendering": compact_preserves_state,
        "state_unchanged_by_json_rendering": machine_preserves_state,
        "state_projection": view.json_value() == expect["state"],
        "compact_rendering": compact == expect["compact_rendering"],
        "renderers_are_distinct": compact != machine,
        "compact_renderer_is_deterministic": compact_is_deterministic,
        "json_renderer_is_deterministic": machine_is_deterministic,
    }
    failed = [name for name, passed in checks.items() if not passed]
    if failed:
        raise AssertionError("failed checks: " + ", ".join(failed))

    return {
        "history": {"event_count": len(decoded_events), "fnv1a64": digest_before},
        "state": view.json_value(),
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
