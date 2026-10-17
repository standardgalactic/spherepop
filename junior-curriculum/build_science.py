import re, shutil, subprocess, sys
from pathlib import Path

root = Path.cwd()
out = root / "pdf"
out.mkdir(exist_ok=True)
wanted = set(sys.argv[1:])
failed = []

for n in range(1, 10):
    if wanted and str(n) not in wanted:
        continue
    bdir = root / f"science-book{n}-source" / "science" / f"book{n}"
    if not bdir.is_dir():
        print(f"Book {n}: folder not found, skipping")
        continue
    mains = [p for p in bdir.glob("*.tex") if "\\documentclass" in p.read_text(errors="ignore")]
    if len(mains) != 1:
        print(f"Book {n}: expected one main .tex, found {len(mains)}, skipping")
        failed.append(n)
        continue
    main = mains[0]
    ok = True
    for _ in range(2):  # twice, for the contents and page references
        r = subprocess.run(["pdflatex", "-interaction=nonstopmode", main.name],
                           cwd=bdir, capture_output=True, text=True)
        if not (bdir / (main.stem + ".pdf")).exists():
            ok = False
            break
    if not ok:
        print(f"Book {n}: build failed (see {main.stem}.log)")
        failed.append(n)
        continue
    title = re.search(r"\\title\{([^}]*)\}", main.read_text(errors="ignore"))
    slug = re.sub(r"[^A-Za-z0-9]+", "-", title.group(1)).strip("-") if title else main.stem
    dest = out / f"Science-Book{n}-{slug}.pdf"
    shutil.copy(bdir / (main.stem + ".pdf"), dest)
    print(f"Book {n}: {dest.name}")

sys.exit(1 if failed else 0)
