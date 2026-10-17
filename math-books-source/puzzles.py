"""Verified puzzle generators for the book build.

Every puzzle is generated from its answer first, then checked by code that does
not share logic with the generator, and fails closed.
"""
import random, re, itertools
import balloon_gen as bg

# ---------------------------------------------------------------- spot the slip
def fmt(n):
    return f"{n:,}"

def build_notebook(a, b, kind):
    """kind: perfect | pop | redraw. Returns (lines, expected_bad_line_numbers)."""
    m, d = (a + b) // 2, (b - a) // 2
    assert a + b == 2 * m and b - a == 2 * d and d >= 1
    sq, cn, ans = m * m, d * d, m * m - d * d
    L = [f"( {a} × {b} )",
         f"redraw  →  ( ( {m} − {d} ) × ( {m} + {d} ) )"]
    bad = []
    if kind == "redraw":
        L.append(f"redraw  →  ( ( {m} × {m} ) − ( {d} × ( {d} + 1 ) ) )")
        L.append(f"pop ( {m} × {m} )  →  {fmt(sq)}")
        c2 = d * (d + 1)
        L.append(f"pop ( {d} × ( {d} + 1 ) )  →  {fmt(c2)}")
        L.append(f"pop ( {fmt(sq)} − {fmt(c2)} )  →  {fmt(sq - c2)}")
        bad = [3]
    else:
        L.append(f"redraw  →  ( ( {m} × {m} ) − ( {d} × {d} ) )")
        L.append(f"pop ( {m} × {m} )  →  {fmt(sq)}")
        if kind == "pop":
            wrong = cn + d  # d x d slip: d + d... gives wrong corner
            if wrong == cn: wrong += 1
            L.append(f"pop ( {d} × {d} )  →  {fmt(wrong)}")
            L.append(f"pop ( {fmt(sq)} − {fmt(wrong)} )  →  {fmt(sq - wrong)}")
            bad = [5]
        else:
            L.append(f"pop ( {d} × {d} )  →  {fmt(cn)}")
            L.append(f"pop ( {fmt(sq)} − {fmt(cn)} )  →  {fmt(ans)}")
    return L, bad

def _norm(s):
    return s.replace(" ", "").replace(",", "")

def check_notebook(lines):
    """Independent trace checker. Returns (bad line numbers, final value, v0)."""
    v0, _ = bg.evaluate(lines[0])
    state, bad = _norm(lines[0]), []
    for i, ln in enumerate(lines[1:], 2):
        if ln.startswith("redraw"):
            expr = ln.split("→", 1)[1]
            if bg.evaluate(expr)[0] != v0:
                bad.append(i)
            state = _norm(expr)
        elif ln.startswith("pop"):
            m = re.match(r"pop (\(.*\))\s+→\s+([\d,]+)\s*$", ln)
            circ, n = m.group(1), int(m.group(2).replace(",", ""))
            if _norm(circ) not in state:
                bad.append(i); continue
            if bg.evaluate(circ)[0] != n:
                bad.append(i)
            state = state.replace(_norm(circ), str(n), 1)
        else:
            raise ValueError(ln)
    final = bg.evaluate(state)[0]
    return bad, final, v0

def make_slip_page(pairs, rng):
    kinds = ["perfect", "pop", "redraw"]
    rng.shuffle(kinds)
    books, keys = [], []
    for (a, b), kind in zip(pairs, kinds):
        lines, expected = build_notebook(a, b, kind)
        bad, final, v0 = check_notebook(lines)
        assert bad == expected, (lines, bad, expected)
        if kind == "perfect":
            assert final == v0
        else:
            assert final != v0
        books.append(lines); keys.append((kind, bad, a, b))
    return books, keys

def slip_tex(books, pairs_info):
    out = []
    names = "ABC"
    for n, lines in zip(names, books):
        out.append("\\begin{tracebox}\n\\begin{Verbatim}[fontsize=\\scriptsize,numbers=left,numbersep=4pt,formatcom=\\color{hudgreen}]")
        out.extend(lines)
        out.append("\\end{Verbatim}\n\\end{tracebox}")
        out.append(f"\\vspace{{0.3em}}\n\\begin{{center}}\\sffamily\\bfseries\\footnotesize Notebook {n}\\ \\ \\ \\ □ perfect \\ \\ □ slip on line \\underline{{\\hspace{{1cm}}}}\\end{{center}}\n")
    return "\n".join(out)

def slip_answers(keys):
    out = []
    for n, (kind, bad, a, b) in zip("ABC", keys):
        m, d = (a + b) // 2, (b - a) // 2
        if kind == "perfect":
            out.append(f"Notebook {n}: \\textbf{{perfect}}. {a} × {b} = {fmt(m*m - d*d)}.")
        elif kind == "pop":
            out.append(f"Notebook {n}: slip on line \\textbf{{5}}. {d} × {d} is {d*d}. The answer should be {fmt(m*m - d*d)}.")
        else:
            out.append(f"Notebook {n}: slip on line \\textbf{{3}}. The corner is {d} × {d}, not {d} × ({d} + 1). The answer should be {fmt(m*m - d*d)}.")
    return "\\par\n".join(out)

# ---------------------------------------------------------------- color by answer
BITMAPS = {
 "robot": ["00011000","00011000","01111110","11011011","11111111","11111111","01100110","01100110"],
 "heart": ["01100110","11111111","11111111","11111111","01111110","00111100","00011000","00000000"],
 "house": ["00011000","00111100","01111110","11111111","11111111","11100111","11100111","11100111"],
}

def _good(T):
    out = set()
    for a in range(2, 200):
        if T % a == 0 and T // a >= 2:
            out.add(f"{a} × {T//a}")
    for m in range(2, 140):
        for dd in range(1, m):
            if m * m - dd * dd == T: out.add(f"{m} × {m} − {dd} × {dd}")
    for a in range(2, 140):
        for dd in range(1, 40):
            for bb in [a + 2 * dd]:
                if a * bb + dd * dd == T: out.add(f"{a} × {bb} + {dd} × {dd}")
    for a in range(2, 40):
        if T - a >= 2 and a < T - a and T > 20: out.add(f"{T - a} + {a}")
        out.add(f"{T + a} − {a}")
    return sorted(out)

def _near(T, rng):
    pool = set()
    for dlt in list(range(-12, 0)) + list(range(1, 13)):
        V = T + dlt
        if V < 4: continue
        for a in range(2, 140):
            if V % a == 0 and V // a >= 2:
                pool.add(f"{a} × {V//a}")
        for m in range(2, 140):
            for dd in range(1, m):
                if m * m - dd * dd == V: pool.add(f"{m} × {m} − {dd} × {dd}")
        for a in range(2, 30):
            pool.add(f"{V - a} + {a}") if V - a >= 2 else None
    return sorted(pool)

def make_color_page(T, bitmap_name, rng):
    bm = BITMAPS[bitmap_name]
    good, near = _good(T), _near(T, rng)
    assert len(good) >= 6, (T, len(good))
    cells = []
    for r, row in enumerate(bm):
        for c, ch in enumerate(row):
            e = rng.choice(good if ch == "1" else near)
            cells.append((r, c, e, ch))
    # independent verification
    for r, c, e, ch in cells:
        v = bg.evaluate(e)[0]
        assert (v == T) == (ch == "1"), (e, v, T)
    return cells

def color_tex(cells):
    cs = 1.3
    L = []
    for r, c, e, ch in cells:
        L.append(f"\\node[draw=black!70,minimum size={cs}cm,text width={cs-0.15:.2f}cm,align=center,inner sep=0pt,font={{\\fontsize{{5.6}}{{6.4}}\\selectfont\\sffamily}}] at ({c*cs:.2f},{-r*cs:.2f}) {{{bg.pretty(e)}}};")
    return "\n".join(L)

def color_answer_tex(bitmap_name):
    bm = BITMAPS[bitmap_name]
    return "\n".join(f"\\fill[hudnavy] ({c*0.4},{-r*0.4}) rectangle +(0.4,-0.4);"
                     for r, row in enumerate(bm) for c, ch in enumerate(row) if ch == "1")
