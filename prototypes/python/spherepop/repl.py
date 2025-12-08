from __future__ import annotations
import sys
from .core import default_collapse, eval_term
from .parser import parse


PROMPT = "spherepop> "


def repl() -> None:
    strategy = default_collapse
    print("Spherepop REPL (Ctrl-D to exit)")
    while True:
        try:
            line = input(PROMPT)
        except EOFError:
            print()
            break
        line = line.strip()
        if not line:
            continue
        if line in {":q", ":quit", ":exit"}:
            break
        try:
            term = parse(line)
            result = eval_term(strategy, term)
            print(result)
        except Exception as e:  # noqa: BLE001
            print(f"error: {e}", file=sys.stderr)


if __name__ == "__main__":
    repl()
