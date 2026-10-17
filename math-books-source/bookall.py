import sys, os, re, random, glob, shutil, subprocess
sys.path.insert(0, ".")
import convert as cv, balloon_gen as bg, puzzles as pz

STAGING = "/tmp/claude-0/-home-claude/da252b08-a280-583d-adfa-9ed8865518a5/scratchpad/staging"
BOOKS = {
 1: ("01-a-useful-wrong-answer", "A Useful\\\\ Wrong Answer", "Estimates that know how wrong they are", "times"),
 3: ("03-two-boxes-one-answer", "Two Boxes,\\\\ One Answer", "Boxes that sound right, and boxes that are right", "corner"),
 4: ("04-the-order-of-discovery", "The Order\\\\ of Discovery", "From messy notebooks to tidy proofs", "corner"),
 5: ("05-reversing-the-calculation", "Reversing the\\\\ Calculation", "Winding a calculation back", "corner"),
 6: ("06-what-must-remain-unresolved", "What Must Remain\\\\ Unresolved", "Saying what you do know", "corner"),
 7: ("07-the-choice-before-the-calculation", "The Choice Before\\\\ the Calculation", "Choosing your tools before you start", "corner"),
 8: ("08-the-boundary-of-a-step", "The Boundary\\\\ of a Step", "Counting steps, and what they cost", "corner"),
 9: ("09-solving-by-changing-the-question", "Solving by Changing\\\\ the Question", "Changing the question, keeping the answer", "corner"),
}
MSGS = ["CHECK YOUR WORK!","KEEP THE NOTEBOOK!","NAME THE UNKNOWN!","TRY A SMALL CASE!","ONE LINE AT A TIME!",
        "ASK IT ANOTHER WAY!","SAY WHAT YOU KNOW!","SPLIT THEN BALANCE!","UNDO IT STEP BY STEP!","BE KIND TO ZERO!"]
TARGETS = [(1225,"heart"),(2491,"house"),(2496,"robot"),(899,"heart"),(1599,"house"),(3599,"robot"),(2400,"heart"),(1764,"house"),(2475,"robot")]
PAIRS = [[(47,53),(29,31),(98,102)],[(38,42),(24,26),(46,54)],[(15,17),(35,45),(58,62)],
         [(44,56),(27,33),(95,105)],[(37,43),(19,21),(64,76)],[(23,27),(45,55),(88,112)]]

def trust_note(path):
    ch = cv.parse_chapter(path)
    for h, b in ch["sections"]:
        if h.lower().startswith("what makes"):
            sent = re.split(r"(?<=[.!?])\s", b.strip().replace("\n", " "))[0]
            return cv.pandoc(sent).strip().replace("\n", " ")
    return None

def make_plan(bn, n, profile):
    k = (n - 1 + bn) % 3
    if k == 0: return {"kind": "slip", "pairs": PAIRS[(n + bn) % len(PAIRS)]}
    if k == 1:
        T, pic = TARGETS[(n + 2 * bn) % len(TARGETS)]
        return {"kind": "color", "target": T, "picture": pic}
    for off in range(len(MSGS)):
        msg = MSGS[(n + bn + off) % len(MSGS)]
        try:
            letters, words = bg.parse_message(msg)
            ex = bg.generate(bg.to_numbers(letters), profile, 100 + n)
            if not bg.verify(msg, ex, profile):
                return {"kind": "balloon", "message": msg, "profile": profile, "riddle": "What is Multix's advice?"}
        except Exception:
            continue
    T, pic = TARGETS[n % len(TARGETS)]
    return {"kind": "color", "target": T, "picture": pic}

def run(bn):
    folder, title, sub, profile = BOOKS[bn]
    src = f"{STAGING}/{folder}"
    out = f"build{bn}"
    os.makedirs(out, exist_ok=True)
    for f in ("preamble.tex", "book_preamble.tex"): shutil.copy(f, out)
    files = sorted(glob.glob(f"{src}/chapter-*.md"), key=lambda f: int(re.search(r"chapter-(\d+)-", f).group(1)))
    extras, plan = {}, {}
    for f in files:
        n = int(re.search(r"chapter-(\d+)-", f).group(1))
        t = trust_note(f)
        extras[n] = {"after": {0: ["\\mnote{\\textbf{Trust check.} " + t + "}\n"]}} if t else {}
        plan[n] = make_plan(bn, n, profile)
    chapters, answers, pzans = cv.build(src, title.replace("\\\\ ", " "), extras, plan, out, seed0=100 * bn)
    nums = sorted(plan)
    back = []
    for i in range(0, len(nums), 2):
        blk = []
        for n in nums[i:i+2]:
            blk.append(f"{{\\bfseries\\large Chapter {n}}}\\par\\smallskip\n{{\\small\n{answers.get(n,'')}\n}}\\par\\smallskip\n{pzans[n]}\\par\\bigskip")
        back.append("\\newpage\n\\rotatebox{180}{\\begin{minipage}[t][\\textheight][t]{\\textwidth}\\sffamily\\footnotesize\n"
                    "{\\bfseries\\large Answers (turn the book around)}\\par\\medskip\n" + "\n".join(blk) + "\n\\end{minipage}}\n")
    titlepg = (r"\thispagestyle{empty}" "\n\\vspace*{1.2cm}\n\\begin{center}\n"
        f"{{\\sffamily\\bfseries\\footnotesize\\color{{hudorange}} BOOK {bn}}}\\\\[6pt]\n"
        f"{{\\sffamily\\bfseries\\fontsize{{24}}{{27}}\\selectfont\\color{{inkbrown}} {title}}}\\\\[10pt]\n"
        f"{{\\sffamily\\small\\color{{hudnavy}} {sub}}}\\\\[1.2cm]\n\\bigmultix\\\\[1.2cm]\n"
        "\\begin{tally}[hudgreen]{HELLO}\nI am Multix. I ride along in every chapter. Watch my readout: it shows what is owed, what is paid, and what is only a guess.\n\\end{tally}\n\\end{center}\n\\newpage\n")
    doc = ("\\documentclass[10.5pt]{article}\n\\input{preamble}\n\\input{book_preamble}\n\\begin{document}\n" + titlepg + "\n".join(chapters) + "\n".join(back) + "\n\\end{document}\n")
    open(f"{out}/book{bn}.tex", "w").write(doc)
    r = subprocess.run(["latexmk", "-xelatex", "-interaction=nonstopmode", "-halt-on-error", f"book{bn}.tex"], cwd=out, capture_output=True, text=True)
    ok = os.path.exists(f"{out}/book{bn}.pdf") and r.returncode == 0
    return ok, plan

if __name__ == "__main__":
    for bn in map(int, sys.argv[1:]):
        try:
            ok, plan = run(bn)
            print(bn, "OK" if ok else "FAIL", [p["kind"][0] for _, p in sorted(plan.items())], flush=True)
        except Exception as e:
            print(bn, "ERR", repr(e)[:300], flush=True)
