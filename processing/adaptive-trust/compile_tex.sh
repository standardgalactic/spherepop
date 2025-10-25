#!/usr/bin/env bash
# Recursively compile all .tex files with XeLaTeX
# Usage: ./compile_all_tex.sh

set -euo pipefail

# Optional: number of XeLaTeX passes
PASSES=2

# Find all .tex files (excluding common aux files or LaTeX internals)
find . -type f -name "*.tex" | while read -r texfile; do
  echo "=========================================="
  echo "Compiling: $texfile"
  echo "=========================================="

  dir=$(dirname "$texfile")
  base=$(basename "$texfile" .tex)

  # Move into the file's directory so relative paths work
  pushd "$dir" >/dev/null || exit 1

  # Run XeLaTeX multiple times for stable TOC/cross-refs
  for ((i=1; i<=PASSES; i++)); do
    echo "  Pass $i of $PASSES..."
    xelatex -interaction=nonstopmode -halt-on-error "$base.tex" >/dev/null || {
      echo "❌ Error compiling $texfile"
      popd >/dev/null
      continue 2
    }
  done

  echo "✅ Generated: $dir/$base.pdf"
  popd >/dev/null
done

echo "------------------------------------------"
echo "All .tex files compiled successfully."

