# Junior Curriculum

A junior curriculum in five subjects: forty-five short illustrated books with stories, experiments and puzzles. Each one teaches how to check, correct and explain your work.

## Status

| Series | Books | State |
| --- | --- | --- |
| Mathematics | 9 | Drafted and typeset (`chapters/`, `pdf/`) |
| Science | 9 | Planned |
| Spelling and Grammar | 9 | Planned |
| Ecology | 9 | Planned |
| Problem Solving | 9 | Planned |

The full plan, with the four-week unit map, is in the [Curriculum Overview](https://standardgalactic.github.io/spherepop/curriculum-overview.pdf)

## Layout

- `chapters/` Markdown source for every chapter, plus the introduction, afterword and instructor preface.
- `tools/` the converter and puzzle generators.
- `overview/` LaTeX source of the curriculum overview (`latexmk -xelatex overview.tex`, using the two preamble files beside it).
- `pdf/` the typeset books.
- `build/` generated output (not tracked).

## Building

Requires python3, pandoc 3.x, XeLaTeX with latexmk, and the DejaVu fonts.

    tools/build_all.sh

Every puzzle is generated from its answer and checked by an independent verifier before it is typeset. Numbers, letters and answer keys are never baked into images.
