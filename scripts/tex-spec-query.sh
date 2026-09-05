#!/usr/bin/env bash
set -euo pipefail

usage() {
  awk '
    /^# tex-spec-query\.sh/ { showing=1 }
    showing && /^#/ {
      line=$0
      sub(/^# ?/, "", line)
      print line
      next
    }
    showing { exit }
  ' "$0"
  exit "${1:-0}"
}

# tex-spec-query.sh — offline specification finder for LaTeX repositories
#
# Usage:
#   ./tex-spec-query.sh [OPTIONS] [ROOT]
#
# ROOT defaults to the current directory.
#
# Options:
#   -o, --output DIR       Report directory (default: ROOT/tex-spec-report)
#   -m, --min-score N      Minimum relevance score (default: 4)
#   -C, --context N        Context lines around matches (default: 2)
#   -q, --query REGEX      Add a case-insensitive extended regular expression
#                         to the built-in specification query
#       --only REGEX       Replace the built-in query with REGEX
#       --include-drafts   Include common generated/build files (aux remains off)
#       --all              Include files even when their score is below threshold
#   -h, --help             Show this help
#
# Examples:
#   ./tex-spec-query.sh ~/src/spherepop
#   ./tex-spec-query.sh -q 'quotient|option space|history' .
#   ./tex-spec-query.sh --only 'Pop|Refuse|Bind|Collapse' -C 4 .
#
# Outputs:
#   summary.txt            Ranked human-readable inventory
#   inventory.tsv           Machine-readable per-file scores and counts
#   matches.txt             Matching passages with line numbers and context
#   duplicate-groups.txt    Byte-identical .tex files grouped by SHA-256
#   files.txt               Every scanned .tex path

root="."
output=""
min_score=4
context=2
extra_query=""
only_query=""
include_drafts=0
show_all=0

while (($#)); do
  case "$1" in
    -o|--output) output=${2:?"missing directory after $1"}; shift 2 ;;
    -m|--min-score) min_score=${2:?"missing number after $1"}; shift 2 ;;
    -C|--context) context=${2:?"missing number after $1"}; shift 2 ;;
    -q|--query) extra_query=${2:?"missing regex after $1"}; shift 2 ;;
    --only) only_query=${2:?"missing regex after $1"}; shift 2 ;;
    --include-drafts) include_drafts=1; shift ;;
    --all) show_all=1; shift ;;
    -h|--help) usage 0 ;;
    --) shift; root=${1:-.}; shift || true; break ;;
    -*) printf 'Unknown option: %s\n' "$1" >&2; usage 2 ;;
    *) root=$1; shift ;;
  esac
done

[[ -d "$root" ]] || { printf 'Not a directory: %s\n' "$root" >&2; exit 2; }
[[ "$min_score" =~ ^[0-9]+$ ]] || { printf 'Invalid minimum score: %s\n' "$min_score" >&2; exit 2; }
[[ "$context" =~ ^[0-9]+$ ]] || { printf 'Invalid context: %s\n' "$context" >&2; exit 2; }

root=$(cd "$root" && pwd -P)
output=${output:-"$root/tex-spec-report"}
mkdir -p "$output"
output=$(cd "$output" && pwd -P)

builtin_query='primitive(s| operators?| operations?| events?)?|event alphabet|kernel state|transition (rule|relation|system)|operational semantics|small[- ]step|reduction rule|typing (rule|judg(e)?ment)|type system|abstract syntax|concrete syntax|grammar|(^|[^A-Za-z])(BNF|EBNF)([^A-Za-z]|$)|desugar(ing)?|formal specification|reference implementation|interpreter|compiler|parser|executable|invariant|determinism|confluence|replay semantics|denotational semantics'
if [[ -n "$only_query" ]]; then
  query=$only_query
elif [[ -n "$extra_query" ]]; then
  query="($builtin_query)|($extra_query)"
else
  query=$builtin_query
fi

tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT
files0="$tmp/files0"

find_args=("$root" -type f -iname '*.tex')
if ((include_drafts == 0)); then
  find_args+=( ! -path '*/.git/*' ! -path '*/tex-spec-report/*' ! -path '*/build/*' ! -path '*/dist/*' )
fi
find "${find_args[@]}" -print0 | sort -z > "$files0"

file_count=$(tr -cd '\0' < "$files0" | wc -c | tr -d ' ')
if ((file_count == 0)); then
  printf 'No .tex files found beneath %s\n' "$root" >&2
  exit 1
fi

: > "$output/files.txt"
while IFS= read -r -d '' file; do
  printf '%s\n' "${file#"$root"/}" >> "$output/files.txt"
done < "$files0"

printf 'sha256\tpath\n' > "$tmp/hashes.tsv"
while IFS= read -r -d '' file; do
  hash=$(sha256sum -- "$file" | awk '{print $1}')
  printf '%s\t%s\n' "$hash" "${file#"$root"/}" >> "$tmp/hashes.tsv"
done < "$files0"

tail -n +2 "$tmp/hashes.tsv" | sort -t $'\t' -k1,1 -k2,2 | awk -F '\t' '
  {
    count[$1]++
    paths[$1] = paths[$1] sprintf("  %s\n", $2)
  }
  END {
    for (h in count)
      if (count[h] > 1)
        printf "%s  (%d files)\n%s", h, count[h], paths[h]
  }
' > "$output/duplicate-groups.txt"

printf 'score\tdefinitions\tresults\tgrammar\tsemantics\ttyping\timplementation\tprimitives\tpath\n' > "$output/inventory.tsv"
: > "$output/matches.txt"

while IFS= read -r -d '' file; do
  rel=${file#"$root"/}
  definitions=$(grep -Eic '\\begin\{(definition|axiom)\}' "$file" || true)
  results=$(grep -Eic '\\begin\{(theorem|proposition|lemma|corollary)\}' "$file" || true)
  grammar=$(grep -Eic 'grammar|(^|[^A-Za-z])(BNF|EBNF)([^A-Za-z]|$)|abstract syntax|concrete syntax' "$file" || true)
  semantics=$(grep -Eic 'operational semantics|denotational semantics|transition (rule|relation|system)|small[- ]step|reduction rule|replay semantics' "$file" || true)
  typing=$(grep -Eic 'typing (rule|judg(e)?ment)|type system|well[- ]typed|type checker' "$file" || true)
  implementation=$(grep -Eic 'reference implementation|implementation strategy|interpreter|compiler|parser|executable|prototype|Haskell|Racket|Python|Rust|Forth' "$file" || true)
  primitives=$({ grep -Eio '\b(Pop|Refuse|Bind|Collapse|Sphere|Merge|Choice|Meld|Open|Reframe|Unlink|SetMeta)\b' "$file" || true; } | wc -l | tr -d ' ')

  # Weight structural evidence more heavily than repeated operator names.
  primitive_cap=$primitives
  ((primitive_cap > 30)) && primitive_cap=30
  score=$((definitions * 3 + results * 2 + grammar * 3 + semantics * 4 + typing * 3 + implementation + primitive_cap))

  printf '%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n' \
    "$score" "$definitions" "$results" "$grammar" "$semantics" "$typing" "$implementation" "$primitives" "$rel" \
    >> "$output/inventory.tsv"

  if ((show_all == 1 || score >= min_score)); then
    {
      printf '\n===== %s (score %d) =====\n' "$rel" "$score"
      grep -Ein -C "$context" -- "$query" "$file" || true
    } >> "$output/matches.txt"
  fi
done < "$files0"

{
  printf 'LaTeX specification query\n'
  printf 'Repository: %s\n' "$root"
  printf 'Files scanned: %d\n' "$file_count"
  printf 'Query: %s\n' "$query"
  printf 'Minimum score: %d\n\n' "$min_score"
  printf 'Ranked candidates\n'
  printf 'Score  Def  Res  Gram  Sem  Type  Impl  Prim  Path\n'
  tail -n +2 "$output/inventory.tsv" |
    sort -t $'\t' -k1,1nr -k9,9 |
    awk -F '\t' -v minimum="$min_score" -v all="$show_all" '
      all || $1 >= minimum {
        printf "%-6s %-4s %-4s %-5s %-4s %-5s %-5s %-5s %s\n", $1,$2,$3,$4,$5,$6,$7,$8,$9
      }
    '
} > "$output/summary.txt"

printf 'Scanned %d .tex files.\n' "$file_count"
printf 'Report: %s\n' "$output/summary.txt"
printf 'Matches: %s\n' "$output/matches.txt"
printf 'Inventory: %s\n' "$output/inventory.tsv"
printf 'Duplicates: %s\n' "$output/duplicate-groups.txt"
