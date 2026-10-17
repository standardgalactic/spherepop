#!/usr/bin/env python3
"""Markdown chapters -> LaTeX book (boxes, Multix asides, puzzle pages)."""
import glob, os, re, subprocess, sys, random, json
import balloon_gen as bg
import puzzles as pz

def pandoc(md):
    if not md.strip(): return ""
    r = subprocess.run(["pandoc", "-f", "markdown+smart", "-t", "latex", "--wrap=none"],
                       input=md, capture_output=True, text=True, check=True)
    t = r.stdout
    t = re.sub(r"\\def\\labelenumi\{\\arabic\{enumi\}\.\}\n?", "", t)
    t = re.sub(r"\\setcounter\{enumi\}\{(\d+)\}\n?", lambda m: m.group(0), t)
    return t

def trace_env(code):
    return "\\begin{tracebox}\n\\begin{Verbatim}[fontsize=\\footnotesize,formatcom=\\color{hudgreen}]\n" + code.rstrip("\n") + "\n\\end{Verbatim}\n\\end{tracebox}\n"

def parse_chapter(path):
    text = open(path).read()
    codes = []
    def grab(m):
        codes.append(m.group(1)); return f"\n\n@@TRACE{len(codes)-1}@@\n\n"
    text = re.sub(r"```\n(.*?)```", grab, text, flags=re.S)
    lines = text.split("\n")
    m = re.search(r"^## Chapter (\d+): (.*)$", text, flags=re.M)
    num, title = int(m.group(1)), m.group(2).strip()
    body = text[m.end():]
    # bridge
    parts = re.split(r"\n---\s*\n", body)
    main, tail = parts[0], "\n".join(parts[1:])
    secs = re.split(r"^### ", main, flags=re.M)
    pre, secs = secs[0], secs[1:]
    sections = []
    for s in secs:
        h, _, b = s.partition("\n")
        sections.append((h.strip(), b))
    return dict(num=num, title=title, pre=pre, sections=sections, tail=tail, codes=codes)

def render_blocks(md, codes):
    """Convert a Markdown fragment; code placeholders become trace panels."""
    blocks = re.split(r"\n\s*\n", md.strip("\n"))
    out, chunk = [], []
    def flush():
        if chunk:
            out.append(pandoc("\n\n".join(chunk))); chunk.clear()
    for b in blocks:
        b = b.strip("\n")
        if not b.strip(): continue
        mt = re.fullmatch(r"@@TRACE(\d+)@@", b.strip())
        if mt:
            flush(); out.append(trace_env(codes[int(mt.group(1))])); continue
        if b.startswith("*For brave readers"):
            flush()
            inner = re.sub(r"^\*For brave readers:\*\s*", "", b)
            out.append("\\begin{braveaside}\n" + pandoc(inner) + "\\end{braveaside}\n"); continue
        if b.startswith("**Predict first:**"):
            flush()
            inner = re.sub(r"^\*\*Predict first:\*\*\s*", "", b)
            out.append("\\begin{predictbox}\n" + pandoc(inner) + "\\end{predictbox}\n"); continue
        chunk.append(b)
    flush()
    return "\n".join(out)

def chapter_tex(book_title, ch, extras, answers_store):
    codes = ch["codes"]
    o = [f"\\chapterbanner{{{book_title.upper()}}}{{{ch['num']}}}{{{ch['title']}}}"]
    pre = ch["pre"].strip("\n")
    reminder = [l[1:].strip() for l in pre.split("\n") if l.startswith(">")]
    story = "\n".join(l for l in pre.split("\n") if not l.startswith(">")).strip()
    if reminder:
        txt = re.sub(r"^\*\*Circle reminder\.\*\*\s*", "", " ".join(reminder))
        o.append("\\begin{tally}[hudgreen]{CIRCLE CHECK}\n" + pandoc(txt).strip() + "\n\\end{tally}\n")
    if story:
        o.append("\\begin{storybox}\n" + render_blocks(story, codes) + "\\end{storybox}\n")
    for i, (h, b) in enumerate(ch["sections"]):
        hl = h.lower()
        if hl.startswith("step"):
            mm = re.match(r"Step (\d+):\s*(.*)", h)
            o.append(f"\\stephead{{{mm.group(1)}}}{{{mm.group(2)}}}")
            o.append(render_blocks(b, codes))
        elif hl.startswith("in circles"):
            o.append("\\chip{IN CIRCLES}")
            o.append(render_blocks(b, codes))
        elif hl.startswith("split, balance"):
            o.append("\\begin{strategybox}\n" + render_blocks(b, codes) + "\\end{strategybox}\n")
        elif hl.startswith("what did we keep"):
            o.append("\\begin{keepbox}\n" + render_blocks(b, codes) + "\\end{keepbox}\n")
        elif hl.startswith("what makes this"):
            title = h.rstrip("?").upper() + "?"
            o.append("\\begin{trustbox}[" + title + "]\n" + render_blocks(b, codes) + "\\end{trustbox}\n")
        elif hl.startswith("practice"):
            o.append("\\begin{practicebox}\n" + render_blocks(b, codes) + "\\end{practicebox}\n")
        elif hl.startswith("answers"):
            answers_store[ch["num"]] = render_blocks(b, codes)
        else:
            o.append(f"\\chip{{{h.upper()}}}\n" + render_blocks(b, codes))
        for ex in extras.get("after", {}).get(i, []):
            o.append(ex)
    # bridge
    bl = [l.strip() for l in ch["tail"].split("\n") if l.strip()]
    if bl:
        o.append("\\begin{bridgebox}\n" + "\n\n".join(pandoc(l) for l in bl) + "\\end{bridgebox}\n")
    out = "\n".join(o)
    for phrase, note in extras.get("footnotes", []):
        assert phrase in out, phrase
        out = out.replace(phrase, phrase + "\\footnote{" + note + "}", 1)
    return out

# ---------------------------------------------------------------- puzzle pages
def puzzle_page(n, spec, seed, outdir):
    rng = random.Random(seed)
    kind = spec["kind"]
    tag = f"pz{n}"
    if kind == "slip":
        books, keys = pz.make_slip_page(spec["pairs"], rng)
        body = pz.slip_tex(books, keys)
        page = ("\\puzzlebanner{Spot the Slip}{Three notebooks. One is perfect. The other two each hide a single slip. Find it!}\n"
                + body + "\n\\begin{tally}[hudamber]{HINT}\nA slip can hide in a \\textbf{pop} (wrong value) or in a \\textbf{redraw} (value changed). Check each line against the problem at the top.\n\\end{tally}\n")
        ans = "\\textbf{Spot the Slip.}\\par\n" + pz.slip_answers(keys)
    elif kind == "color":
        T = spec["target"]
        cells = pz.make_color_page(T, spec["picture"], rng)
        page = (f"\\puzzlebanner{{Color by Answer}}{{Shade every box whose answer is exactly \\textbf{{{T:,}}}. Leave the others blank. What appears?}}\n"
                "\\begin{center}\n\\begin{tikzpicture}[x=1cm,y=1cm]\n" + pz.color_tex(cells) + "\n\\end{tikzpicture}\n\\end{center}\n"
                "\\begin{tally}[hudgreen]{SCAN}\nSome boxes look close and are not. Check the corner on each one.\n\\end{tally}\n")
        ans = (f"\\textbf{{Color by Answer ({T:,}).}} The shaded boxes make a {spec['picture']}.\\par\\smallskip\n"
               "\\begin{tikzpicture}[x=1cm,y=1cm]\n" + pz.color_answer_tex(spec["picture"]) + "\n\\end{tikzpicture}")
    elif kind == "balloon":
        msg = spec["message"]
        letters, words = bg.parse_message(msg)
        target = bg.to_numbers(letters)
        exprs = bg.generate(target, spec.get("profile", "corner"), seed)
        problems = bg.verify(msg, exprs, spec.get("profile", "corner"))
        assert not problems, problems
        values = [int(bg.evaluate(e)[0]) for e in exprs]
        assert bg.decode(values) == letters
        kx, ky = bg.write_scene(exprs, words, f"{outdir}/{tag}_scene.tex")
        bg.write_banner(words, f"{outdir}/{tag}_banner.tex")
        bg.write_answers(exprs, values, msg, f"{outdir}/{tag}_ans.tex")
        key = []
        for row, rng_ in enumerate([range(1, 14), range(14, 27)]):
            for i in rng_:
                key.append(f"\\node[draw=black!60,minimum width=0.74cm,minimum height=0.74cm,inner sep=0pt,font=\\tiny\\sffamily,fill=paper] at ({i-(1 if row==0 else 14)},{-0.9*row}) {{{chr(64+i)}\\,=\\,{i}}};")
        page = (f"\\puzzlebanner{{The Balloon Message}}{{{spec['riddle']} Work out each balloon, use the key to turn the number into a letter, and fill the banner box with the same number.}}\n"
                "\\begin{center}\n\\begin{tikzpicture}[x=0.78cm,y=0.78cm]\n" + "\n".join(key) + "\n\\end{tikzpicture}\n\\end{center}\n"
                "\\begin{center}\n\\begin{tikzpicture}[x=1cm,y=1cm,scale=0.97]\n\\input{" + f"{tag}_scene.tex" + "}\n\\end{tikzpicture}\n\\end{center}\n\\vspace{-0.4em}\n"
                "\\begin{tcolorbox}[enhanced,colback=paper,colframe=inkbrown,boxrule=1.4pt,arc=3mm,left=6pt,right=6pt,top=5pt,bottom=18pt]\n"
                "\\centering {\\sffamily\\bfseries What was the answer?}\\par\\vspace{0.5em}\n\\begin{tikzpicture}[x=1cm,y=1cm]\n\\input{" + f"{tag}_banner.tex" + "}\n\\end{tikzpicture}\n\\end{tcolorbox}\n")
        ans = ("\\textbf{Balloon Message.}\\par\\smallskip\n\\input{" + f"{tag}_ans.tex" + "}\\par\\smallskip Message: \\textbf{\\input{" + f"{tag}_ans_msg.tex" + "}}")
    else:
        raise ValueError(kind)
    return page, ans

def build(book_dir, book_title, extras_all, plan, outdir, seed0=100):
    os.makedirs(outdir, exist_ok=True)
    answers_store, pz_answers, chapters = {}, {}, []
    files = sorted(glob.glob(os.path.join(book_dir, "chapter-*.md")),
                   key=lambda f: int(re.search(r"chapter-(\d+)-", f).group(1)))
    for f in files:
        ch = parse_chapter(f)
        tex = chapter_tex(book_title, ch, extras_all.get(ch["num"], {}), answers_store)
        page, ans = puzzle_page(ch["num"], plan[ch["num"]], seed0 + ch["num"], outdir)
        pz_answers[ch["num"]] = ans
        chapters.append(f"% ---- chapter {ch['num']}\n{tex}\n\\newpage\n{page}\n\\newpage\n")
    return chapters, answers_store, pz_answers
