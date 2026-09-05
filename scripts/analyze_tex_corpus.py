#!/usr/bin/env python3
"""Analyze every tracked .tex file in the repository for normative grammar
content, as distinct from essays that merely use Spherepop terminology.

What this script does (and does NOT do):
  - It reads the actual text of every file `git ls-files '*.tex'` returns —
    not just filenames or a curated document list — and searches it for
    concrete signals of an executable/formal specification: BNF-style
    grammar productions, primitive definitions, transition/reduction rules,
    state-tuple models, typing judgments, desugaring rules, and invariants
    (propositions/theorems/lemmas), plus explicit implementation claims.
  - It hashes every file's content (sha256) so byte-identical duplicates
    (e.g. a document copied to both a numbered draft and a "final" name)
    are collapsed into one entry before any count is reported, per the
    tracking issue's requirement not to let repeated documents inflate the
    apparent number of distinct specifications.
  - It flags the generated adaptive-trust corpus separately: those essays
    live under processing/adaptive-trust/**/Adaptive_Trust_Dynamics_Corpus/
    and/or contain the generator's literal placeholder boilerplate
    ("To be filled.", "To be outlined."), and are not read as Spherepop
    grammar sources regardless of their signal score.
  - It does NOT decide anything by itself about which document is "the"
    canonical grammar. It reports evidence (which signals were found,
    where, how strongly) so a human (or a follow-up review) can make that
    call. The "normative candidate" / "essay" split below is a heuristic
    triage to make that follow-up review tractable, not a final ruling.

Usage:
    python3 scripts/analyze_tex_corpus.py [--json] [--out PATH] [--min-score N]

Output:
    A Markdown report (default: printed to stdout) with:
      1. A summary table of tracked vs. distinct-after-dedup file counts,
         split into generated corpus / normative candidates / essays.
      2. Per-file signal counts for every non-generated file, sorted by
         total signal score, descending.
      3. The duplicate groups that were collapsed (so the dedup itself is
         auditable, not just its resulting count).

    With --json, the same data is emitted as JSON instead (useful for
    feeding into a further review pass), still written to --out or stdout.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Directory fragment that identifies the generated adaptive-trust corpus.
GENERATED_CORPUS_DIR_MARKER = "Adaptive_Trust_Dynamics_Corpus"

# Literal boilerplate the corpus generator leaves in every essay stub; used
# as a content-based fallback in case a generated essay is ever moved out
# of its corpus directory (so classification doesn't depend on path alone).
GENERATED_CORPUS_PLACEHOLDERS = (
    "To be filled.",
    "To be outlined.",
)

# Each signal is (label, compiled regex, weight). Weight is just a relative
# importance used for sorting/triage; it is not a certainty score.
SIGNALS: list[tuple[str, re.Pattern, int]] = [
    (
        "bnf_grammar",
        re.compile(r"::=|\\text\{.*?\}\s*::=|\bBNF\b|\\(?:begin|section\*?)\{.*?[Gg]rammar"),
        3,
    ),
    (
        "primitive_defs",
        re.compile(
            r"\\(?:begin\{definition\}|textbf\{Definition)|"
            r"\b(?:Pop|Refuse|Bind|Collapse)\b\s*[:(]|"
            r"\\text\{(?:Pop|Refuse|Bind|Collapse)\}"
        ),
        3,
    ),
    (
        "transition_rules",
        re.compile(r"\\vdash|\\Rightarrow|\breduction\b|\btransition\b|\brewrit(?:e|ing)\b", re.I),
        2,
    ),
    (
        "state_model",
        re.compile(r"\\sigma\s*=|\bstate\s+tuple\b|\bkernel\s+state\b|\\Omega\b", re.I),
        2,
    ),
    (
        "typing_judgment",
        re.compile(r"\\Gamma\s*\\vdash|\btyping\s+judgment\b|\bjudgment\b", re.I),
        2,
    ),
    (
        "desugaring",
        re.compile(r"\bdesugar(?:s|ed|ing)?\b|\bsyntactic\s+sugar\b", re.I),
        2,
    ),
    (
        "invariants",
        re.compile(r"\\begin\{(?:proposition|theorem|lemma|corollary)\}|\\textbf\{(?:Proposition|Theorem|Lemma|Corollary)", re.I),
        1,
    ),
    (
        "implementation_claim",
        re.compile(r"\bimplements?\b|\bexecutable\b|\btest_spherepop\b|\binterpreter\b|\bkernel\b", re.I),
        1,
    ),
]

# Below this total weighted score, a non-generated file is triaged as an
# "essay" (uses terminology, no concrete grammar/formal apparatus found).
DEFAULT_MIN_SCORE = 4


@dataclass
class FileReport:
    path: str
    sha256: str
    size: int
    signal_counts: dict[str, int] = field(default_factory=dict)
    score: int = 0
    is_generated_corpus: bool = False
    classification: str = ""  # "generated_corpus" | "normative_candidate" | "essay"


def tracked_tex_files() -> list[str]:
    # -z + NUL-splitting avoids git's default quoting/escaping of
    # non-ASCII filenames (e.g. accented characters), which otherwise
    # silently produces paths that don't exist on disk.
    out = subprocess.run(
        ["git", "ls-files", "-z", "*.tex"],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return [p for p in out.stdout.split("\0") if p]


def sha256_of(path: Path) -> tuple[str, int]:
    h = hashlib.sha256()
    data = path.read_bytes()
    h.update(data)
    return h.hexdigest(), len(data)


def read_text_best_effort(path: Path) -> str:
    data = path.read_bytes()
    for encoding in ("utf-8", "latin-1"):
        try:
            return data.decode(encoding)
        except UnicodeDecodeError:
            continue
    return data.decode("utf-8", errors="replace")


def is_generated_corpus(relpath: str, text: str) -> bool:
    if GENERATED_CORPUS_DIR_MARKER in relpath:
        return True
    return any(p in text for p in GENERATED_CORPUS_PLACEHOLDERS)


def score_file(text: str) -> tuple[dict[str, int], int]:
    counts: dict[str, int] = {}
    total = 0
    for label, pattern, weight in SIGNALS:
        n = len(pattern.findall(text))
        counts[label] = n
        if n:
            total += weight * min(n, 5)  # cap per-signal contribution
    return counts, total


def analyze(min_score: int) -> tuple[list[FileReport], dict[str, list[str]]]:
    files = tracked_tex_files()
    by_hash: dict[str, list[str]] = defaultdict(list)
    reports_by_hash: dict[str, FileReport] = {}

    for rel in files:
        p = REPO_ROOT / rel
        if not p.exists():
            continue  # tracked but missing in working tree; skip rather than crash
        digest, size = sha256_of(p)
        by_hash[digest].append(rel)
        if digest in reports_by_hash:
            continue  # already scored the first copy; duplicates share its report
        text = read_text_best_effort(p)
        generated = is_generated_corpus(rel, text)
        counts, score = score_file(text)
        if generated:
            classification = "generated_corpus"
        elif score >= min_score:
            classification = "normative_candidate"
        else:
            classification = "essay"
        reports_by_hash[digest] = FileReport(
            path=rel,
            sha256=digest,
            size=size,
            signal_counts=counts,
            score=score,
            is_generated_corpus=generated,
            classification=classification,
        )

    duplicate_groups = {h: paths for h, paths in by_hash.items() if len(paths) > 1}
    # Order reports: representative path is the first tracked path seen for that hash.
    ordered = [reports_by_hash[h] for h in by_hash.keys()]
    return ordered, duplicate_groups


def render_markdown(reports: list[FileReport], duplicate_groups: dict[str, list[str]], min_score: int) -> str:
    total_tracked = sum(len(v) for v in duplicate_groups.values()) + sum(
        1 for r in reports if r.sha256 not in duplicate_groups
    )
    distinct = len(reports)
    generated = [r for r in reports if r.classification == "generated_corpus"]
    normative = [r for r in reports if r.classification == "normative_candidate"]
    essays = [r for r in reports if r.classification == "essay"]

    lines = []
    lines.append("# Tex corpus grammar-content analysis")
    lines.append("")
    lines.append(
        "Generated by `scripts/analyze_tex_corpus.py`. Every tracked `.tex` file's "
        "actual content was scanned for grammar/formal-specification signals; "
        "byte-identical files were collapsed by sha256 before classification. "
        "See the script's docstring for exactly what is and isn't claimed here."
    )
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- Tracked `.tex` files: **{total_tracked}**")
    lines.append(f"- Distinct files after byte-identical dedup: **{distinct}**")
    lines.append(f"- Generated adaptive-trust corpus essays (excluded from triage): **{len(generated)}**")
    lines.append(
        f"- Normative-candidate documents (weighted signal score \u2265 {min_score}): **{len(normative)}**"
    )
    lines.append(f"- Essays / terminology-only documents (below threshold): **{len(essays)}**")
    lines.append(f"- Duplicate groups collapsed: **{len(duplicate_groups)}**")
    lines.append("")

    def table(title: str, rows: list[FileReport]):
        out = [f"## {title}", ""]
        if not rows:
            out.append("_None._")
            out.append("")
            return out
        out.append("| Path | Score | " + " | ".join(label for label, _, _ in SIGNALS) + " | sha256 (short) |")
        out.append("|---" * (2 + len(SIGNALS)) + "|")
        for r in sorted(rows, key=lambda r: r.score, reverse=True):
            sig_cells = " | ".join(str(r.signal_counts.get(label, 0)) for label, _, _ in SIGNALS)
            out.append(f"| `{r.path}` | {r.score} | {sig_cells} | `{r.sha256[:10]}` |")
        out.append("")
        return out

    lines += table("Normative-candidate documents", normative)
    lines += table("Essays / terminology-only documents", essays)

    lines.append("## Generated adaptive-trust corpus")
    lines.append("")
    lines.append(
        f"{len(generated)} distinct files matched the generated-corpus directory marker "
        f"(`{GENERATED_CORPUS_DIR_MARKER}`) or generator placeholder text. Listed by "
        "directory only, not individually, since they are not read as grammar sources:"
    )
    lines.append("")
    by_dir: dict[str, int] = defaultdict(int)
    for r in generated:
        by_dir[str(Path(r.path).parent)] += 1
    for d, n in sorted(by_dir.items()):
        lines.append(f"- `{d}/`: {n} file(s)")
    lines.append("")

    lines.append("## Duplicate groups (byte-identical, collapsed above)")
    lines.append("")
    if not duplicate_groups:
        lines.append("_None found._")
    else:
        for h, paths in sorted(duplicate_groups.items(), key=lambda kv: kv[1][0]):
            lines.append(f"- sha256 `{h[:10]}` ({len(paths)} copies):")
            for p in paths:
                lines.append(f"  - `{p}`")
    lines.append("")

    return "\n".join(lines)


def render_json(reports: list[FileReport], duplicate_groups: dict[str, list[str]], min_score: int) -> str:
    payload = {
        "min_score": min_score,
        "signals": [label for label, _, _ in SIGNALS],
        "files": [
            {
                "path": r.path,
                "sha256": r.sha256,
                "size": r.size,
                "signal_counts": r.signal_counts,
                "score": r.score,
                "is_generated_corpus": r.is_generated_corpus,
                "classification": r.classification,
            }
            for r in reports
        ],
        "duplicate_groups": duplicate_groups,
    }
    return json.dumps(payload, indent=2, sort_keys=True)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", action="store_true", help="emit JSON instead of Markdown")
    ap.add_argument("--out", type=Path, default=None, help="write report to this path instead of stdout")
    ap.add_argument(
        "--min-score",
        type=int,
        default=DEFAULT_MIN_SCORE,
        help=f"weighted signal score threshold for 'normative_candidate' (default: {DEFAULT_MIN_SCORE})",
    )
    args = ap.parse_args()

    reports, duplicate_groups = analyze(args.min_score)
    text = render_json(reports, duplicate_groups, args.min_score) if args.json else render_markdown(
        reports, duplicate_groups, args.min_score
    )

    if args.out:
        args.out.write_text(text)
    else:
        sys.stdout.write(text)
        if not text.endswith("\n"):
            sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
