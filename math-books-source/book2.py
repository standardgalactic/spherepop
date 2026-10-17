import sys, os, re, random
sys.path.insert(0, ".")
import convert as cv

SRC = "/tmp/claude-0/-home-claude/da252b08-a280-583d-adfa-9ed8865518a5/scratchpad/staging/02-the-work-that-vanishes"
OUT = "build2"

def tally(label, text, eye="hudgreen"):
    return f"\\begin{{tally}}[{eye}]{{{label}}}\n{text}\n\\end{{tally}}\n"
def note(t):
    return "\\mnote{" + t + "}\n"

EXTRAS = {
 1: {"after": {0: [note("\\textbf{Try it!} Cut a 12 by 8 rectangle out of grid paper. Then do the slide yourself.")],
               1: [tally("SCAN", "Provisional answer after the square: 100, with 4 still owed. After the last pop: nothing owed.", "hudamber")]}},
 2: {"after": {0: [note("\\textbf{Test it.} Try $65\\times65$. Does the answer end in 25?")],
               1: [tally("SCAN", "A square is a rectangle plus a corner. Check on a small one: $6\\times6=4\\times8+2\\times2=32+4=36$.")]}},
 3: {"after": {0: [note("\\textbf{Your turn.} Try a rope of 20 m. Which rectangle holds the most?")],
               1: [tally("SCAN", "Same sum, different product. The square always holds the most. Check: $7\\times13=91$ is less than $10\\times10=100$.")]}},
 4: {"after": {0: [note("\\textbf{Near 100?} Count how far under each number is. Those are the shortfalls.")],
               1: [tally("SCAN", "Two strips overlap in one corner. Count that corner once. For $96\\times94$ the corner is $4\\times6=24$.", "hudamber")]}},
 5: {"after": {0: [note("\\textbf{Choose first.} Four tricks, one product. Pick before you calculate.")],
               2: [tally("SCAN", "Count the pops and how friendly each one is. Three easy pops can beat three hard ones.")]}},
 6: {"after": {0: [note("\\textbf{Count places.} One place times one place gives two places.")],
               3: [tally("WARNING", "$0.2\\times0.2$ is $0.04$. Not $0.4$. Not $0.004$.", "hudred")]}},
 7: {"after": {0: [note("\\textbf{Next square.} From $n\\times n$, add $n+(n+1)$.")],
               1: [tally("SCAN", "$25\\times25=625$. The border for $26\\times26$ is $25+26=51$. So $26\\times26=676$.")]}},
 8: {"after": {0: [note("\\textbf{Case file.} $48\\times52$ appears in every book.")],
               1: [tally("SCAN", "The cross pieces cancel only when both numbers are the same distance from the middle.", "hudamber")]}},
}

PLAN = {
 1: {"kind": "slip", "pairs": [(47,53),(29,31),(98,102)]},
 2: {"kind": "color", "target": 1225, "picture": "heart"},
 3: {"kind": "balloon", "message": "SQUARES WIN!", "riddle": "What did all the gardens agree on?"},
 4: {"kind": "slip", "pairs": [(38,42),(24,26),(46,54)]},
 5: {"kind": "color", "target": 2491, "picture": "house"},
 6: {"kind": "balloon", "message": "COUNT THE PLACES!", "riddle": "What is the first rule for decimals?"},
 7: {"kind": "slip", "pairs": [(15,17),(35,45),(58,62)]},
 8: {"kind": "color", "target": 2496, "picture": "robot"},
}

chapters, answers, pzans = cv.build(SRC, "The Work That Vanishes", EXTRAS, PLAN, OUT)

# back-of-book answers, rotated, two chapters per page
groups = [(1,2),(3,4),(5,6),(7,8)]
back = []
for g in groups:
    blk = []
    for n in g:
        blk.append(f"{{\\bfseries\\large Chapter {n}}}\\par\\smallskip\n{{\\small\n{answers.get(n,'')}\n}}\\par\\smallskip\n{pzans[n]}\\par\\bigskip")
    back.append("\\newpage\n\\rotatebox{180}{\\begin{minipage}[t][\\textheight][t]{\\textwidth}\\sffamily\\footnotesize\n"
                "{\\bfseries\\large Answers (turn the book around)}\\par\\medskip\n" + "\n".join(blk) + "\n\\end{minipage}}\n")

title = r"""
\thispagestyle{empty}
\vspace*{1.2cm}
\begin{center}
{\sffamily\bfseries\footnotesize\color{hudorange} BOOK 2}\\[6pt]
{\sffamily\bfseries\fontsize{26}{28}\selectfont\color{inkbrown} The Work\\ That Vanishes}\\[10pt]
{\sffamily\small\color{hudnavy} The missing corner, and why it works}\\[1.2cm]
\bigmultix\\[1.2cm]
\begin{tally}[hudgreen]{HELLO}
I am Multix. I ride along in every chapter. Watch my readout: it shows what is owed, what is paid, and what is only a guess.
\end{tally}
\end{center}
\newpage
"""
doc = (r"\documentclass[10.5pt]{article}" "\n\\input{preamble}\n\\input{book_preamble}\n\\begin{document}\n"
       + title + "\n".join(chapters) + "\n".join(back) + "\n\\end{document}\n")
open(f"{OUT}/book2.tex", "w").write(doc)
print("wrote", len(doc), "chars;", len(chapters), "chapters")
