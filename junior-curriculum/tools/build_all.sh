#!/bin/sh
# Builds all nine Mathematics books into ../build/bookN/bookN.pdf
# Requires: python3, pandoc 3.x, xelatex + latexmk, DejaVu fonts, TeX Gyre/DejaVu Math.
cd "$(dirname "$0")"
python3 book2.py
python3 bookall.py 1 3 4 5 6 7 8 9
