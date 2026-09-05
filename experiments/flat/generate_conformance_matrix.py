#!/usr/bin/env python3
"""Generates `CONFORMANCE.md`: a per-fixture, per-implementation matrix.

This is the Phase D deliverable "publish a generated conformance
matrix" from the tracking issue. It runs every available adapter
against every fixture in `fixtures/` and records PASS / FAIL / SKIP
(manual fixture) / BLOCKED (adapter could not be exercised at all, with
a concrete, investigated reason — never silently omitted).

Usage:
    python3 generate_conformance_matrix.py
"""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import run_python  # noqa: E402  (local module, see run_python.py)

FIXTURES_DIR = HERE / "fixtures"
KERNEL_DIR = HERE.parent.parent / "spherepop-kernel"
COMPILER_DIR = HERE.parent.parent / "compiler"
COMPILER_BUILD_DIR = COMPILER_DIR / "build"
GO_DIR = HERE.parent.parent / "spherepop-go"

# Implementations that cannot currently be exercised at all, with the
# concrete reason established by direct investigation (not assumed).
# See IMPLEMENTATIONS.md for the full write-up of each.
#
# SphereForth (prototypes/sphereforth_gforth.zip) is intentionally not
# listed here: it now runs (via a bare-kernel gforth workaround, see
# IMPLEMENTATIONS.md) and its `demo.fs`/REPLAY were verified end-to-end
# after fixing seven pre-existing bugs. It's still absent from the
# table above because no fixture-format adapter exists for it yet (its
# vocabulary is close to, but not identical to, the canonical JSON
# fixture schema) -- see CONFORMANCE.md's "Blocked adapters" section,
# which is retained as a home for this note even though the adapter
# itself isn't blocked anymore.
BLOCKED_ADAPTERS = {
}


def run_python_adapter() -> dict[str, str]:
    results: dict[str, str] = {}
    for path in sorted(FIXTURES_DIR.glob("*.json")):
        name = path.stem
        fixture = json.loads(path.read_text())
        if fixture.get("manual"):
            results[name] = "SKIP"
            continue
        try:
            failures = run_python.run_fixture(path)
        except Exception as e:
            results[name] = f"FAIL ({e!r})"
            continue
        results[name] = "PASS" if not failures else "FAIL"
    return results


def run_rust_adapter() -> dict[str, str]:
    results: dict[str, str] = {}
    try:
        proc = subprocess.run(
            ["cargo", "run", "--quiet", "--bin", "fixtures"],
            cwd=KERNEL_DIR,
            capture_output=True,
            text=True,
            timeout=120,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired) as e:
        # No usable `cargo`/linker in this environment -- record as blocked
        # rather than silently omitting the Rust row.
        for path in sorted(FIXTURES_DIR.glob("*.json")):
            results[path.stem] = f"BLOCKED ({e})"
        return results

    for line in proc.stdout.splitlines():
        parts = line.split(None, 1)
        if len(parts) == 2 and parts[0] in ("PASS", "FAIL", "SKIP"):
            results[parts[1].split(" ")[0]] = parts[0]
    return results


def run_c_adapter() -> dict[str, str]:
    """Runs compiler/'s `sp_fixtures` binary -- a standalone C port of
    the canonical event/history/arbiter model (see
    compiler/tools/fixtures/kernel.h), independent of compiler/'s
    general-purpose Bubble-based interpreter, whose own Bind/Collapse
    are still known to diverge from the canonical semantics."""
    results: dict[str, str] = {}
    binary = COMPILER_BUILD_DIR / "sp_fixtures"

    if not binary.exists():
        try:
            COMPILER_BUILD_DIR.mkdir(exist_ok=True)
            subprocess.run(
                ["cmake", ".."], cwd=COMPILER_BUILD_DIR,
                capture_output=True, text=True, timeout=120, check=True,
            )
            subprocess.run(
                ["make", "sp_fixtures"], cwd=COMPILER_BUILD_DIR,
                capture_output=True, text=True, timeout=120, check=True,
            )
        except (FileNotFoundError, subprocess.TimeoutExpired, subprocess.CalledProcessError) as e:
            # No usable C toolchain (cmake/make/cc) configured in this
            # environment -- record as blocked rather than silently
            # omitting the C row. (In this sandbox, building requires
            # CC=<path to zig-cc wrapper> on PATH; see README.md.)
            for path in sorted(FIXTURES_DIR.glob("*.json")):
                results[path.stem] = f"BLOCKED ({e})"
            return results

    try:
        proc = subprocess.run(
            [str(binary), str(FIXTURES_DIR)],
            capture_output=True, text=True, timeout=60,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired) as e:
        for path in sorted(FIXTURES_DIR.glob("*.json")):
            results[path.stem] = f"BLOCKED ({e})"
        return results

    for line in proc.stdout.splitlines():
        parts = line.split(None, 1)
        if len(parts) == 2 and parts[0] in ("PASS", "FAIL", "SKIP"):
            results[parts[1].split(" ")[0]] = parts[0]
    return results


def run_go_adapter() -> dict[str, str]:
    """Runs spherepop-go's `cmd/fixtures` runner -- an independent,
    from-scratch Go reimplementation of the canonical event/history/
    arbiter model (see spherepop-go/README.md), sharing no code with
    the Rust kernel, the Python oracle, or compiler/."""
    results: dict[str, str] = {}
    try:
        proc = subprocess.run(
            ["go", "run", "./cmd/fixtures", str(FIXTURES_DIR)],
            cwd=GO_DIR,
            capture_output=True,
            text=True,
            timeout=120,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired) as e:
        # No usable `go` toolchain in this environment -- record as
        # blocked rather than silently omitting the Go row.
        for path in sorted(FIXTURES_DIR.glob("*.json")):
            results[path.stem] = f"BLOCKED ({e})"
        return results

    for line in proc.stdout.splitlines():
        parts = line.split(None, 1)
        if len(parts) == 2 and parts[0] in ("PASS", "FAIL", "SKIP"):
            results[parts[1].split(" ")[0]] = parts[0]
    return results


def main() -> int:
    fixture_names = sorted(p.stem for p in FIXTURES_DIR.glob("*.json"))

    py_results = run_python_adapter()
    rs_results = run_rust_adapter()
    go_results = run_go_adapter()
    c_results = run_c_adapter()

    lines = [
        "# Generated conformance matrix",
        "",
        "Generated by `experiments/flat/generate_conformance_matrix.py`. "
        "Do not hand-edit -- re-run the script instead.",
        "",
        "| Fixture | Rust kernel | Python oracle | Go implementation | C kernel |",
        "|---|---|---|---|---|",
    ]
    any_fail = False
    for name in fixture_names:
        rs = rs_results.get(name, "?")
        py = py_results.get(name, "?")
        go = go_results.get(name, "?")
        c = c_results.get(name, "?")
        if rs not in ("PASS", "SKIP") or py not in ("PASS", "SKIP") or go not in ("PASS", "SKIP") or c not in ("PASS", "SKIP"):
            any_fail = True
        lines.append(f"| {name} | {rs} | {py} | {go} | {c} |")

    lines += ["", "## Blocked adapters", ""]
    for impl, reason in BLOCKED_ADAPTERS.items():
        lines.append(f"### {impl}")
        lines.append("")
        lines.append(reason)
        lines.append("")

    out_path = HERE / "CONFORMANCE.md"
    out_path.write_text("\n".join(lines) + "\n")
    print(f"Wrote {out_path}")
    return 1 if any_fail else 0


if __name__ == "__main__":
    raise SystemExit(main())
