#!/usr/bin/env python3
"""Balloon Message generator.

Order of work: message -> target numbers -> expressions with those exact values
-> independent verification -> only then write LaTeX fragments.

    python3 balloon_gen.py --message "LET'S STICK TOGETHER!" --profile corner --seed 7
"""
import argparse, ast, itertools, json, random, sys
from fractions import Fraction

# ---------------------------------------------------------------- profiles
# An operation is "introduced" for a book when its profile allows it.
PROFILES = {
    "plus-minus": {"ops": {"+", "-"},           "max_level": 1},
    "times":      {"ops": {"+", "-", "*"},      "max_level": 2},
    "brackets":   {"ops": {"+", "-", "*"},      "max_level": 3},
    "corner":     {"ops": {"+", "-", "*"},      "max_level": 4},
}

# ---------------------------------------------------------------- message
def parse_message(msg):
    """Return (letters, layout). Punctuation is typography, not arithmetic."""
    words, cur, marks = [], [], []
    for ch in msg:
        if ch.isalpha():
            cur.append(ch.upper())
        elif ch == " ":
            if cur: words.append({"letters": "".join(cur), "marks": marks}); cur, marks = [], []
        else:
            marks.append((len(cur), ch))
    if cur or marks:
        words.append({"letters": "".join(cur), "marks": marks})
    letters = "".join(w["letters"] for w in words)
    assert letters.isalpha() and letters.isascii() and letters.isupper()
    return letters, words

def to_numbers(letters):
    return [ord(c) - 64 for c in letters]

def decode(values):
    assert all(isinstance(v, int) and 1 <= v <= 26 for v in values), values
    return "".join(chr(64 + v) for v in values)

# ---------------------------------------------------------------- evaluator
def _ev(node, ops_seen):
    if isinstance(node, ast.Expression):
        return _ev(node.body, ops_seen)
    if isinstance(node, ast.Constant):
        if type(node.value) is not int:
            raise ValueError("only whole numbers allowed")
        if node.value <= 0:
            raise ValueError("zero and negative operands are not allowed")
        return Fraction(node.value)
    if isinstance(node, ast.BinOp):
        sym = {ast.Add: "+", ast.Sub: "-", ast.Mult: "*"}.get(type(node.op))
        if sym is None:
            raise ValueError("operation not allowed: " + type(node.op).__name__)
        ops_seen.add(sym)
        a, b = _ev(node.left, ops_seen), _ev(node.right, ops_seen)
        return a + b if sym == "+" else a - b if sym == "-" else a * b
    raise ValueError("syntax not allowed: " + type(node).__name__)

def parse(expr):
    src = expr.replace("×", "*").replace("−", "-").replace(",", "").strip()
    return ast.parse(src, mode="eval")

def evaluate(expr):
    ops = set()
    v = _ev(parse(expr), ops)
    return v, ops

# ---------------------------------------------------------------- levels
def _kind(n):
    return type(n.op).__name__ if isinstance(n, ast.BinOp) else "Num"

def _num(n):
    return n.value if isinstance(n, ast.Constant) else None

def level(expr):
    """1 plain sum/difference, 2 product, 3 brackets, 4 distribution/correction."""
    t = parse(expr).body
    # level 4 patterns
    if isinstance(t, ast.BinOp) and isinstance(t.op, (ast.Sub, ast.Add)):
        l, r = t.left, t.right
        if isinstance(l, ast.BinOp) and isinstance(r, ast.BinOp) and isinstance(l.op, ast.Mult) and isinstance(r.op, ast.Mult):
            lf, rf = {_num(l.left), _num(l.right)}, {_num(r.left), _num(r.right)}
            # square minus corner
            if isinstance(t.op, ast.Sub) and _num(l.left) == _num(l.right) and _num(r.left) == _num(r.right):
                return 4
            # common factor (distribution)
            if lf & rf:
                return 4
        # ten-times minus one: 10 x b - b
        if isinstance(t.op, ast.Sub) and isinstance(l, ast.BinOp) and isinstance(l.op, ast.Mult) \
                and _num(l.left) == 10 and _num(l.right) == _num(r) and _num(r) is not None:
            return 4
    if isinstance(t, ast.BinOp) and isinstance(t.op, ast.Mult):
        if _kind(t.left) in ("Add", "Sub") or _kind(t.right) in ("Add", "Sub"):
            return 3
    if isinstance(t, ast.BinOp) and isinstance(t.op, (ast.Add, ast.Sub)):
        if _kind(t.left) in ("Add", "Sub") or _kind(t.right) in ("Add", "Sub"):
            return 3
        if isinstance(t.right, ast.BinOp) and isinstance(t.op, ast.Sub):
            return 3
    # level 2: contains a product
    if any(isinstance(n, ast.BinOp) and isinstance(n.op, ast.Mult) for n in ast.walk(t)):
        return 2
    return 1

# ---------------------------------------------------------------- generation
def candidates(lv):
    """Yield (expression string) for each level."""
    out = []
    if lv == 1:
        for a in range(2, 31):
            for b in range(2, 31):
                out.append(f"{a} + {b}")
                if a > b: out.append(f"{a} − {b}")
    elif lv == 2:
        for a in range(2, 13):
            for b in range(2, 13):
                out.append(f"{a} × {b}")
                if a * b >= 10:
                    for c in range(2, 10):
                        out.append(f"{a} × {b} + {c}")
                        out.append(f"{a} × {b} − {c}")
    elif lv == 3:
        for a in range(2, 16):
            for b in range(2, 16):
                for c in range(2, 12):
                    out.append(f"({a} + {b}) − {c}")
                    out.append(f"({a} + {b}) × {c}")
                    if a - b >= 2: out.append(f"({a} − {b}) × {c}")
                    out.append(f"{a} × ({b} + {c})")
                    if b - c >= 2: out.append(f"{a} × ({b} − {c})")
    elif lv == 4:
        for m in range(3, 15):
            for d in range(1, m):
                out.append(f"{m} × {m} − {d} × {d}")
        for a in range(2, 13):
            for b in range(2, 13):
                for c in range(1, 13):
                    if b - c >= 2 and c >= 2: out.append(f"{a} × {b} − {a} × {c}")
                    if c >= 2: out.append(f"{a} × {b} + {a} × {c}")
        for b in range(2, 10):
            out.append(f"10 × {b} − {b}")
    return out

def l4pattern(e):
    t = parse(e).body
    l, r = t.left, t.right
    if isinstance(l, ast.BinOp) and isinstance(r, ast.BinOp) and isinstance(l.op, ast.Mult) and isinstance(r.op, ast.Mult):
        if isinstance(t.op, ast.Sub) and _num(l.left) == _num(l.right) and _num(r.left) == _num(r.right):
            return "corner"
        return "distribution"
    return "tenminus"

L4_ORDER = ["corner", "distribution", "corner", "tenminus"]

def tier_for(i, n):
    """Progressive difficulty: quarters of the sequence, last quarter heaviest."""
    f = i / n
    return 1 if f < 0.25 else 2 if f < 0.5 else 3 if f < 0.75 else 4

def generate(values, profile, seed):
    rng = random.Random(seed)
    maxlv = PROFILES[profile]["max_level"]
    pools = {lv: candidates(lv) for lv in range(1, maxlv + 1)}
    exprs, used = [], set()
    l4_count = 0
    for i, v in enumerate(values):
        lv = min(tier_for(i, len(values)), maxlv)
        for tryl in range(lv, 0, -1):
            pool = [e for e in pools[tryl] if e not in used and evaluate(e)[0] == v and level(e) == tryl]
            if pool and tryl == 4:
                want = L4_ORDER[l4_count % len(L4_ORDER)]
                pref = [e for e in pool if l4pattern(e) == want]
                pool = pref or pool
            if pool:
                e = rng.choice(pool)
                if tryl == 4: l4_count += 1
                break
        else:
            raise SystemExit(f"cannot build expression for value {v}")
        used.add(e); exprs.append(e)
    return exprs

# ---------------------------------------------------------------- verification
def verify(message, exprs, profile):
    """Independent check. Returns list of problems (empty means pass)."""
    problems = []
    letters, _ = parse_message(message)
    target = to_numbers(letters)
    prof = PROFILES[profile]
    if len(exprs) != len(target):
        problems.append(f"{len(exprs)} balloons for {len(target)} letters")
        return problems
    values, levels = [], []
    for i, e in enumerate(exprs, 1):
        try:
            v, ops = evaluate(e)
        except Exception as ex:
            problems.append(f"balloon {i}: {e!r} rejected: {ex}"); continue
        if v.denominator != 1:
            problems.append(f"balloon {i}: {e!r} is not a whole number"); continue
        v = int(v)
        if not ops <= prof["ops"]:
            problems.append(f"balloon {i}: {e!r} uses operation(s) {sorted(ops - prof['ops'])} not allowed in profile {profile}")
        lv = level(e)
        if lv > prof["max_level"]:
            problems.append(f"balloon {i}: {e!r} has level {lv} above profile maximum {prof['max_level']}")
        values.append(v); levels.append(lv)
    if len(set(exprs)) != len(exprs):
        problems.append("duplicate expressions on the page")
    if levels and any(b < a for a, b in zip(levels, levels[1:])):
        problems.append(f"difficulty is not non-decreasing: {levels}")
    if not problems:
        got = decode(values)
        if got != letters:
            problems.append(f"decodes to {got!r}, expected {letters!r}")
        if values != target:
            problems.append("values differ from target sequence")
    return problems

# ---------------------------------------------------------------- LaTeX output
COLORS = ["hudred", "balloonorange", "hudamber", "balloongreen", "balloonblue", "balloonpurple", "balloonpink"]

def esc(s):
    return s.replace("×", r"$\times$").replace("−", r"$-$").replace("+", r"$+$")

def pretty(e):
    """Break long expressions at the last top-level + or - so they fit a balloon."""
    if len(e) <= 10:
        return r"\mbox{" + esc(e) + "}"
    depth, cut = 0, None
    for i, ch in enumerate(e):
        depth += ch == "("; depth -= ch == ")"
        if depth == 0 and ch in "+−" and i > 0:
            cut = i
    if cut is None:
        return r"\mbox{" + esc(e) + "}"
    return r"\mbox{" + esc(e[:cut].strip()) + r"}\\\mbox{" + esc(e[cut:].strip()) + "}"

def write_scene(exprs, words, path):
    n = len(exprs)
    per_row = 6
    rows = [exprs[i:i + per_row] for i in range(0, n, per_row)]
    xs = 1.8
    kx, ky = (per_row - 1) * xs / 2, -(len(rows) * 2.55) - 0.3
    L = []
    pos = []
    k = 0
    for r, row in enumerate(rows):
        off = (per_row - len(row)) * xs / 2
        for c, e in enumerate(row):
            pos.append((off + c * xs + (0.35 if r % 2 else 0), -r * 2.55, k)); k += 1
    # strings first (behind balloons)
    for x, y, k in pos:
        L.append(f"\\draw[black!55,line width=0.5pt] ({x:.2f},{y-0.98:.2f}) .. controls ({x+0.2*(-1)**k:.2f},{y-1.9:.2f}) and ({kx+(k-n/2)*0.04:.2f},{ky+1.1:.2f}) .. ({kx:.2f},{ky+0.1:.2f});")
    # bow at the tie point
    L.append(f"\\fill[hudorange] ({kx:.2f},{ky+0.1:.2f}) -- ({kx-0.55:.2f},{ky+0.35:.2f}) -- ({kx-0.45:.2f},{ky-0.1:.2f}) -- cycle;")
    L.append(f"\\fill[hudorange] ({kx:.2f},{ky+0.1:.2f}) -- ({kx+0.55:.2f},{ky+0.35:.2f}) -- ({kx+0.45:.2f},{ky-0.1:.2f}) -- cycle;")
    L.append(f"\\fill[inkbrown] ({kx:.2f},{ky+0.1:.2f}) circle (0.1);")
    for x, y, k in pos:
        col = COLORS[k % len(COLORS)]
        L.append(f"\\begin{{scope}}[shift={{({x:.2f},{y:.2f})}}]")
        L.append(f"\\fill[{col}!85!white,draw={col}!60!black,line width=0.7pt] (0,0) ellipse (0.78 and 0.98);")
        L.append(f"\\fill[{col}!60!black] (-0.12,-0.98) -- (0.12,-0.98) -- (0,-1.12) -- cycle;")
        L.append(f"\\fill[white,opacity=0.55,rotate=25] (-0.38,0.45) ellipse (0.12 and 0.22);")
        L.append(f"\\node[circle,draw=black!70,fill=white,inner sep=1pt,font=\\tiny\\bfseries\\sffamily] at (-0.55,0.8) {{{k+1}}};")
        L.append(f"\\node[text width=1.2cm,align=center,font=\\scriptsize\\sffamily,inner sep=0pt,text=black] at (0,-0.02) {{{pretty(exprs[k])}}};")
        L.append("\\end{scope}")
    open(path, "w").write("\n".join(L))
    return kx, ky

def write_banner(words, path):
    bw, gap = 0.52, 0.45
    x = 0.0
    L, num = [], 1
    total = sum(len(w["letters"]) for w in words) * bw + (len(words) - 1) * gap
    x0 = -total / 2
    x = x0
    for wi, w in enumerate(words):
        for li in range(len(w["letters"])):
            for pos, ch in w["marks"]:
                if pos == li and ch != "!":
                    L.append(f"\\node[font=\\Large\\bfseries,anchor=south] at ({x-0.01:.2f},{0.55:.2f}) {{{ch if ch != "'" else chr(92)+"textquoteright{}"}}};")
            L.append(f"\\draw[fill=white,line width=0.8pt] ({x:.2f},0) rectangle ({x+bw:.2f},0.6);")
            L.append(f"\\node[font=\\tiny\\sffamily] at ({x+bw/2:.2f},-0.2) {{{num}}};")
            x += bw; num += 1
        for pos, ch in w["marks"]:
            if pos == len(w["letters"]) and ch == "!":
                L.append(f"\\node[font=\\Large\\bfseries] at ({x+0.15:.2f},0.3) {{!}};")
        x += gap
    open(path, "w").write("\n".join(L))
    return total

def write_answers(exprs, values, message, path):
    letters, words = parse_message(message)
    rows = []
    for i, (e, v) in enumerate(zip(exprs, values), 1):
        rows.append(f"{i}.&{esc(e)}&$=$ {v}&{chr(64+v)}\\\\")
    shown = message.replace("'", "\\textquotesingle{}")
    body = ("\\begin{tabular}{@{}rllc@{}}\n" + "\n".join(rows) + "\n\\end{tabular}")
    open(path, "w").write(body)
    open(path.replace(".tex", "_msg.tex"), "w").write(shown)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--message", default="LET'S STICK TOGETHER!")
    ap.add_argument("--profile", default="corner", choices=PROFILES)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--out", default=".")
    a = ap.parse_args()
    letters, words = parse_message(a.message)
    target = to_numbers(letters)
    exprs = generate(target, a.profile, a.seed)
    problems = verify(a.message, exprs, a.profile)
    if problems:
        print("VERIFICATION FAILED:"); [print(" -", p) for p in problems]; sys.exit(1)
    values = [int(evaluate(e)[0]) for e in exprs]
    print("target :", " ".join(map(str, target)))
    print("values :", " ".join(map(str, values)))
    print("decoded:", decode(values))
    print("levels :", [level(e) for e in exprs])
    for i, e in enumerate(exprs, 1): print(f"  {i:2d}  {e}  = {values[i-1]}")
    write_scene(exprs, words, f"{a.out}/balloon_scene.tex")
    write_banner(words, f"{a.out}/balloon_banner.tex")
    write_answers(exprs, values, a.message, f"{a.out}/balloon_answers.tex")
    json.dump({"message": a.message, "profile": a.profile, "seed": a.seed, "exprs": exprs, "values": values},
              open(f"{a.out}/balloon_data.json", "w"), ensure_ascii=False, indent=1)
    print("VERIFIED. LaTeX fragments written.")

if __name__ == "__main__":
    main()
