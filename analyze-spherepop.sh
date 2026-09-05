#!/usr/bin/env bash

# analyze-spherepop.sh (v2)
#
# Hierarchical, resumable repository analysis using Ollama / Granite.
# Same overall shape as v1:
#
#   repository -> extraction -> canonicalization -> document summaries
#       -> thematic cluster syntheses -> cross-corpus synthesis
#       -> reflexive analysis -> adversarial critique
#       -> reconstruction -> final theory report
#
# What changed from v1, and why (see analysis/ for the full writeup):
#
#   - CANONICALIZATION now runs before clustering. Draft versions of
#     the same document (X.tex / X-v01.tex / X-draft-01.tex) are
#     grouped and only the canonical one feeds synthesis, so draft
#     evolution can't be mistaken for corpus contradiction. Review
#     manifest/plan.json (entries with "needs_review": true) before
#     letting the run proceed past that stage.
#
#   - Chunk summarization is POSITION-AWARE: every chunk prompt
#     carries the document title, "chunk K of N", and a running
#     one-paragraph abstract carried forward from earlier chunks, so
#     chunks are never summarized in total isolation from the rest of
#     their own document.
#
#   - Every summary claim requires a quoted source citation, and a
#     mechanical (non-LLM) groundedness check strips any claim whose
#     quote doesn't actually appear in the source. This runs after
#     every chunk summary and every document reduction.
#
#   - Cluster and cross-corpus synthesis happen in BATCHES with an
#     explicit "synthesis so far + new batch" merge prompt, instead
#     of one single-shot call over the whole cluster/corpus. Read the
#     running synthesis after any batch to see if it's drifting
#     before the run continues.
#
#   - Caching is content-hashed (model + prompt-template version +
#     exact prompt text) inside pipeline_functions.py, not
#     existence-only, so editing a prompt template invalidates only
#     what it actually affects. FORCE=1 still forces a full rerun.
#
# All prompt construction, chunking, hashing, and groundedness logic
# lives in pipeline_functions.py, next to this script. This file only
# orchestrates: build the manifest, call each stage in order, loop
# over documents/batches. Nothing here builds a prompt inline.
#
# Usage:
#
#   chmod +x analyze-spherepop.sh
#   ./analyze-spherepop.sh
#
# Force regeneration:
#
#   FORCE=1 ./analyze-spherepop.sh
#
# Models / batch size may be overridden:
#
#   FAST_MODEL=granite4.1:3b \
#   DEEP_MODEL=granite4.1:8b \
#   BATCH_SIZE=6 \
#   ./analyze-spherepop.sh

set -Eeuo pipefail
shopt -s nullglob   # unmatched globs (e.g. a cluster with 0 members, or a
                    # stage that produced 0 batches) expand to nothing
                    # instead of the literal pattern string, which would
                    # otherwise be treated as a real (nonexistent) filename.

###############################################################################
# Configuration
###############################################################################

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYLIB="${ROOT}/pipeline_functions.py"

ANALYSIS="${ROOT}/analysis"

MANIFEST_DIR="${ANALYSIS}/00-manifest"
TEXT_DIR="${ANALYSIS}/01-extracted"
SUMMARY_DIR="${ANALYSIS}/02-document-summaries"
CLUSTER_DIR="${ANALYSIS}/03-cluster-syntheses"
CROSS_DIR="${ANALYSIS}/04-cross-analysis"
REFLECTION_DIR="${ANALYSIS}/05-reflection"
CRITIQUE_DIR="${ANALYSIS}/06-critique"
RECONSTRUCTION_DIR="${ANALYSIS}/07-reconstruction"
FINAL_DIR="${ANALYSIS}/08-final"

CACHE_DIR="${ANALYSIS}/.cache"
LOG_FILE="${ANALYSIS}/analysis.log"

FAST_MODEL="${FAST_MODEL:-granite4.1:3b}"
DEEP_MODEL="${DEEP_MODEL:-granite4.1:8b}"

FORCE="${FORCE:-0}"

CHUNK_CHARS="${CHUNK_CHARS:-45000}"
BATCH_SIZE="${BATCH_SIZE:-6}"

mkdir -p \
    "$MANIFEST_DIR" "$TEXT_DIR" "$SUMMARY_DIR" "$CLUSTER_DIR" \
    "$CROSS_DIR" "$REFLECTION_DIR" "$CRITIQUE_DIR" \
    "$RECONSTRUCTION_DIR" "$FINAL_DIR" "$CACHE_DIR"

# Self-marker: any directory anywhere under ROOT carrying this file is
# a pipeline OUTPUT tree — this run's own analysis/, or an archived
# copy of a previous run that got moved or renamed elsewhere in the
# repo — and must never be re-ingested as corpus content. The marker
# travels with the directory even if it's later renamed/moved, so
# archiving an old run under a new name doesn't require remembering
# to also update an exclusion list somewhere.
touch "${ANALYSIS}/.pipeline-output"

touch "$LOG_FILE"

###############################################################################
# Utilities
###############################################################################

log() {
    printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*" >> "$LOG_FILE"
}

die() {
    echo "ERROR: $*" >&2
    exit 1
}

pyrun() {
    python3 "$PYLIB" "$@"
}

command -v ollama  >/dev/null || die "ollama not found"
command -v python3 >/dev/null || die "python3 not found"

###############################################################################
# Manifest
###############################################################################

echo "Building repository manifest..."

# Directory NAMES that are never corpus content, wherever they occur in
# the tree — this catches the current archived-analysis situation
# immediately (v01/analysis/... is excluded because it's named
# "analysis", regardless of where "v01" sits), plus common CI/tooling
# directories that showed up in the manifest but aren't research
# content. Adjust this list for your repo if needed.
EXCLUDE_DIR_NAMES=(
    .git __pycache__ analysis
    .github .pytest_cache .venv venv
    node_modules dist build .tox .mypy_cache .ruff_cache
)

# Glob-style directory NAME patterns, matched the same way as above
# but allowing wildcards — covers your versioned archive convention
# (v01-analysis, v02-analysis, ...) without needing the marker file
# or a manual touch after every future archive/rename.
EXCLUDE_DIR_GLOBS=(
    '*-analysis'
)

# Any directory anywhere under ROOT carrying the pipeline's own output
# marker (see above) is excluded too, regardless of its name — this is
# what makes a *renamed* archive (not just one named "analysis") safe
# in future runs without editing this script.
mapfile -t OUTPUT_TREES < <(find "$ROOT" -type f -name '.pipeline-output' 2>/dev/null | xargs -r -n1 dirname | sort -u)

PRUNE_ARGS=()
for name in "${EXCLUDE_DIR_NAMES[@]}"; do
    PRUNE_ARGS+=( ! -path "*/${name}/*" ! -path "*/${name}" )
done
for glob in "${EXCLUDE_DIR_GLOBS[@]}"; do
    PRUNE_ARGS+=( ! -path "*/${glob}/*" ! -path "*/${glob}" )
done
for dir in "${OUTPUT_TREES[@]}"; do
    PRUNE_ARGS+=( ! -path "${dir}/*" ! -path "${dir}" )
done

if [[ "${#OUTPUT_TREES[@]}" -gt 0 ]]; then
    echo "Excluding ${#OUTPUT_TREES[@]} marked pipeline-output tree(s) from the manifest:"
    printf '  %s\n' "${OUTPUT_TREES[@]}"
fi

find "$ROOT" -type f \
    "${PRUNE_ARGS[@]}" \
    ! -name '*.pyc' \
    | sort > "${MANIFEST_DIR}/all-files.txt"

find "$ROOT" -type f -name '*.tex' \
    "${PRUNE_ARGS[@]}" \
    | sort > "${MANIFEST_DIR}/tex-files.txt"

find "$ROOT" -type f \( -name '*.md' -o -name '*.txt' \) \
    "${PRUNE_ARGS[@]}" \
    | sort > "${MANIFEST_DIR}/prose-files.txt"

find "$ROOT/spherepop" "$ROOT/tests" -type f -name '*.py' \
    "${PRUNE_ARGS[@]}" \
    2>/dev/null | sort > "${MANIFEST_DIR}/python-files.txt"

cat "${MANIFEST_DIR}/tex-files.txt" "${MANIFEST_DIR}/prose-files.txt" \
    > "${MANIFEST_DIR}/versionable-files.txt"

###############################################################################
# Canonicalization — runs BEFORE extraction/summarization
###############################################################################

PLAN_JSON="${MANIFEST_DIR}/plan.json"

echo
echo "============================================================"
echo " CANONICALIZATION"
echo "============================================================"
echo

if [[ ! -s "$PLAN_JSON" || "$FORCE" == 1 ]]; then
    pyrun canonicalize "$ROOT" "${MANIFEST_DIR}/versionable-files.txt" "$PLAN_JSON"
    NEEDS_REVIEW=$(python3 -c "
import json
plan = json.load(open('${PLAN_JSON}'))
flagged = [d for d in plan['logical_documents'] if d.get('needs_review')]
for d in flagged:
    print(f\"  {d['stem']}: chose {d['canonical']!r} ({d['reason']})\")
print(len(flagged))
" )
    echo "$NEEDS_REVIEW"
    echo
    echo "Review any 'needs_review' picks above (or in ${PLAN_JSON}) before"
    echo "continuing — edit plan.json directly to override a canonical pick."
else
    echo "[cached] $PLAN_JSON"
fi

###############################################################################
# LaTeX / prose extraction (unchanged from v1 — this stage was not the
# problem; kept close to the original strategy: Pandoc, existing PDF,
# compiled PDF, then a conservative source-cleanup fallback)
###############################################################################

extract_tex() {
    local src="$1"
    local rel="${src#$ROOT/}"
    local slug out
    slug="$(python3 -c "import sys; sys.path.insert(0,'$ROOT'); from pipeline_functions import slugify; print(slugify(sys.argv[1]))" "$rel")"
    out="${TEXT_DIR}/${slug}.txt"

    [[ -s "$out" && "$FORCE" != 1 ]] && return

    echo "Extracting: $rel"

    if command -v pandoc >/dev/null; then
        if pandoc --from=latex --to=plain --wrap=none "$src" > "${out}.partial" 2>/dev/null; then
            if [[ -s "${out}.partial" ]]; then
                mv "${out}.partial" "$out"
                return
            fi
        fi
    fi

    local pdf="${src%.tex}.pdf"
    if [[ -f "$pdf" ]] && command -v pdftotext >/dev/null; then
        if pdftotext -layout "$pdf" "${out}.partial" 2>/dev/null && [[ -s "${out}.partial" ]]; then
            mv "${out}.partial" "$out"
            return
        fi
    fi

    if command -v latexmk >/dev/null && command -v pdftotext >/dev/null; then
        local build="${CACHE_DIR}/pdf-${slug}"
        mkdir -p "$build"
        if latexmk -pdf -interaction=nonstopmode -halt-on-error \
            -output-directory="$build" "$src" >/dev/null 2>&1
        then
            local generated="${build}/$(basename "${src%.tex}.pdf")"
            if [[ -f "$generated" ]] && pdftotext -layout "$generated" "${out}.partial" && [[ -s "${out}.partial" ]]; then
                mv "${out}.partial" "$out"
                return
            fi
        fi
    fi

    sed \
        -e 's/%.*$//' \
        -e 's/\\section{\([^}]*\)}/\n\1\n/g' \
        -e 's/\\subsection{\([^}]*\)}/\n\1\n/g' \
        -e 's/\\subsubsection{\([^}]*\)}/\n\1\n/g' \
        "$src" > "$out"
}

echo
echo "============================================================"
echo " EXTRACTION"
echo "============================================================"
echo

while IFS= read -r file; do
    extract_tex "$file"
done < "${MANIFEST_DIR}/tex-files.txt"

while IFS= read -r src; do
    rel="${src#$ROOT/}"
    slug="$(python3 -c "import sys; sys.path.insert(0,'$ROOT'); from pipeline_functions import slugify; print(slugify(sys.argv[1]))" "$rel")"
    out="${TEXT_DIR}/${slug}.txt"
    [[ ! -s "$out" || "$FORCE" == 1 ]] && cp "$src" "$out"
done < "${MANIFEST_DIR}/prose-files.txt"

###############################################################################
# Resolve canonical set — only these feed summarization/clustering.
# Superseded drafts stay extracted on disk for manual inspection but
# are excluded from every downstream synthesis stage.
###############################################################################

CANONICAL_LIST="${MANIFEST_DIR}/canonical-summaries-input.txt"
SUPERSEDED_LIST="${MANIFEST_DIR}/superseded-excluded.txt"

pyrun resolve-canonical "$PLAN_JSON" "$ROOT" "$TEXT_DIR" "$CANONICAL_LIST" "$SUPERSEDED_LIST"

###############################################################################
# Per-document summarization (position-aware, groundedness-checked)
###############################################################################

summarize_document() {
    local src="$1"
    local base title doc_dir final_summary
    base="$(basename "$src" .txt)"
    title="$base"
    doc_dir="${SUMMARY_DIR}/${base}"
    final_summary="${doc_dir}/summary.md"

    if [[ -s "$final_summary" && "$FORCE" != 1 ]]; then
        echo "[cached] $final_summary"
        return
    fi

    mkdir -p "$doc_dir"
    echo "Summarizing: $base"
    log "summarize_document base=${base}"

    local chunks_dir="${doc_dir}/chunks"
    local n_chunks
    n_chunks="$(pyrun chunk-file "$src" "$chunks_dir" --chunk-chars "$CHUNK_CHARS")"

    local running_abstract="${doc_dir}/running-abstract.txt"
    rm -f "$running_abstract"

    local i=1
    for chunk_file in "$chunks_dir"/*.txt; do
        local tag; tag="$(printf '%04d' "$i")"

        local abstract_prompt="${doc_dir}/chunk-${tag}-abstract-prompt.txt"
        pyrun build-abstract-prompt "$title" "$chunk_file" "$running_abstract" "$abstract_prompt"
        pyrun call-model "$FAST_MODEL" "$abstract_prompt" "$running_abstract" --cache-dir "$CACHE_DIR" ${FORCE:+--force}

        local chunk_prompt="${doc_dir}/chunk-${tag}-prompt.txt"
        local chunk_raw="${doc_dir}/chunk-${tag}-summary.raw.md"
        local chunk_summary="${doc_dir}/chunk-${tag}-summary.md"

        pyrun build-chunk-prompt "$title" "$i" "$n_chunks" \
            "cluster synthesis and cross-corpus synthesis" \
            "$chunk_file" "$running_abstract" "$chunk_prompt"
        pyrun call-model "$FAST_MODEL" "$chunk_prompt" "$chunk_raw" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
        pyrun check-groundedness "$chunk_raw" "$chunk_file" "$chunk_summary"

        i=$((i + 1))
    done

    local reduction_prompt="${doc_dir}/reduction-prompt.txt"
    local reduction_raw="${doc_dir}/summary.raw.md"

    pyrun build-reduction-prompt "$title" \
        "cluster synthesis and cross-corpus synthesis" \
        "$doc_dir" "$reduction_prompt"
    pyrun call-model "$FAST_MODEL" "$reduction_prompt" "$reduction_raw" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
    pyrun check-groundedness "$reduction_raw" "$src" "$final_summary"
}

echo
echo "============================================================"
echo " DOCUMENT ANALYSIS (canonical documents only — see ${SUPERSEDED_LIST}"
echo " for drafts excluded from synthesis)"
echo "============================================================"
echo

while IFS= read -r src; do
    [[ -n "$src" ]] || continue
    summarize_document "$src"
done < "$CANONICAL_LIST"

###############################################################################
# Thematic cluster synthesis — batched, rolling merge
###############################################################################

make_cluster() {
    local name="$1"
    local regex="$2"

    local cluster_out="${CLUSTER_DIR}/${name}.md"
    if [[ -s "$cluster_out" && "$FORCE" != 1 ]]; then
        echo "[cached] $cluster_out"
        return
    fi

    local members="${CLUSTER_DIR}/${name}-members.txt"
    : > "$members"
    while IFS= read -r src; do
        [[ -n "$src" ]] || continue
        local base; base="$(basename "$src" .txt)"
        local summary="${SUMMARY_DIR}/${base}/summary.md"
        [[ -s "$summary" ]] || continue
        if [[ "$base" =~ $regex ]]; then
            echo "$summary" >> "$members"
        fi
    done < "$CANONICAL_LIST"

    if [[ ! -s "$members" ]]; then
        echo "[skip] $name: no matching canonical documents"
        return
    fi

    local batch_dir="${CLUSTER_DIR}/${name}-batches"
    local n_batches
    n_batches="$(pyrun batch-plan "$members" "$BATCH_SIZE" "$batch_dir")"

    if [[ "$n_batches" -eq 0 ]]; then
        echo "[skip] $name: batch-plan produced 0 batches"
        return
    fi

    local b=1
    for batch_file in "$batch_dir"/*.txt; do
        local prompt="${CLUSTER_DIR}/${name}-batch-${b}-prompt.txt"
        pyrun build-batch-merge-prompt \
            "cluster '${name}'" "$b" "$n_batches" \
            "$cluster_out" "$batch_file" "$prompt"
        pyrun call-model "$DEEP_MODEL" "$prompt" "$cluster_out" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
        b=$((b + 1))
    done
}

echo
echo "============================================================"
echo " THEMATIC SYNTHESIS (batches of ${BATCH_SIZE} documents/batch)"
echo "============================================================"
echo

make_cluster "identity-history"       'identity|history|event-history|execution-history|forkability'
make_cluster "admissibility-refusal"  'admissib|refus|commitment|irreversib|collapse'
make_cluster "geometry-dynamics"      'geodesic|geometry|scope|trajectory|rotation|dynamics'
make_cluster "computation"            'comput|spherepop-os|specification|python|haskell|racket|implementation'
make_cluster "memory-attention"       'memory|attention|intelligence|thought|semantic'
make_cluster "textbook-foundations"   'textbook|foundation|ecology|truth|logic|language|distinction'
make_cluster "adaptive-trust"         'adaptive-trust|cycle1|cycle2|renewal|diagnosis'
make_cluster "history-development"    'history-of-spherepop|changelog|future|improvement|theory-status'

###############################################################################
# Cross-corpus synthesis — batched over cluster syntheses
###############################################################################

echo
echo "============================================================"
echo " CROSS-CORPUS SYNTHESIS"
echo "============================================================"
echo

CROSS_OUTPUT="${CROSS_DIR}/spherepop-synthesis.md"

if [[ -s "$CROSS_OUTPUT" && "$FORCE" != 1 ]]; then
    echo "[cached] $CROSS_OUTPUT"
else
    CLUSTER_LIST="${CROSS_DIR}/cluster-members.txt"
    find "$CLUSTER_DIR" -maxdepth 1 -name '*.md' \
        ! -name '*-batches' | sort > "$CLUSTER_LIST"

    CROSS_BATCH_DIR="${CROSS_DIR}/batches"
    n_batches="$(pyrun batch-plan "$CLUSTER_LIST" "$BATCH_SIZE" "$CROSS_BATCH_DIR")"

    if [[ "$n_batches" -eq 0 ]]; then
        die "cross-corpus synthesis: no cluster syntheses were produced (every cluster matched 0 documents) — check plan.json and the cluster regexes in this script"
    fi

    b=1
    for batch_file in "$CROSS_BATCH_DIR"/*.txt; do
        prompt="${CROSS_DIR}/batch-${b}-prompt.txt"
        pyrun build-batch-merge-prompt \
            "the full corpus" "$b" "$n_batches" \
            "$CROSS_OUTPUT" "$batch_file" "$prompt"
        pyrun call-model "$DEEP_MODEL" "$prompt" "$CROSS_OUTPUT" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
        b=$((b + 1))
    done
fi

###############################################################################
# Reflexive pass / adversarial critique / reconstruction / final report
#
# These four stages are single-shot over already-synthesized,
# already-groundedness-checked material, so they keep the v1 shape —
# one prompt, one call — but built via pyrun instead of an inline
# heredoc, and still content-hash cached.
###############################################################################

run_single_stage() {
    local stage_name="$1" prompt_body_file="$2" input_file="$3" \
          extra_input_label="$4" extra_input_file="$5" output="$6"

    if [[ -s "$output" && "$FORCE" != 1 ]]; then
        echo "[cached] $output"
        return
    fi

    local prompt="${output}.prompt.txt"
    {
        cat "$prompt_body_file"
        cat "$input_file"
        if [[ -n "$extra_input_file" && -s "$extra_input_file" ]]; then
            echo; echo; echo "$extra_input_label"; echo "${extra_input_label//?/=}"; echo
            cat "$extra_input_file"
        fi
    } > "$prompt"

    pyrun call-model "$DEEP_MODEL" "$prompt" "$output" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
}

echo
echo "============================================================"
echo " REFLEXIVE ANALYSIS / CRITIQUE / RECONSTRUCTION / FINAL REPORT"
echo "============================================================"
echo

REFLECTION_BODY="${REFLECTION_DIR}/prompt-body.txt"
cat > "$REFLECTION_BODY" <<'PROMPT'
Read the following attempted synthesis of Spherepop reflexively.

Every claim below carries a [source: "..."] citation that has
already been mechanically verified against its source document —
treat uncited or flagged [UNGROUNDED ...] lines as unreliable and
do not build on them.

Do not ask merely whether the synthesis is correct. Ask what
conceptual machinery the synthesis itself had to use in order to
make the corpus coherent.

Identify: concepts functioning as hidden primitives; circular
definitions; concepts doing several incompatible jobs; structures
that recur at multiple scales; operations more fundamental than the
nouns used to describe them; distinctions lost during synthesis;
ideas clearer when considered historically; ideas weaker when
considered historically; places where implementation is more precise
than prose; places where prose contains commitments absent from
implementation.

Then propose a more economical conceptual basis for Spherepop.

SYNTHESIS
=========

PROMPT
REFLECTION_OUTPUT="${REFLECTION_DIR}/reflexive-analysis.md"
run_single_stage "reflection" "$REFLECTION_BODY" "$CROSS_OUTPUT" "" "" "$REFLECTION_OUTPUT"

CRITIQUE_BODY="${CRITIQUE_DIR}/prompt-body.txt"
cat > "$CRITIQUE_BODY" <<'PROMPT'
Act as a technically serious skeptical reviewer of the following
Spherepop synthesis and reflexive analysis.

Every claim in the material below has already been mechanically
checked against its source document; treat it as reliably grounded
unless marked [UNGROUNDED ...].

Do not dismiss the project merely because its terminology is
unusual. Instead identify exact failure modes: undefined primitives;
equivocation; circularity; category errors; claims stronger than
their formal support; mathematical statements lacking necessary
assumptions; accidental rediscovery of known structures;
implementation behavior contradicting prose; examples that do not
establish the claimed general result; terminology that obscures
simpler formulations; unfalsifiable claims; missing counterexamples;
missing invariants; places where multiple theories have been joined
without a demonstrated bridge.

For each substantial criticism, state what would be required to
repair it, and cite which specific document(s) the criticism applies
to rather than the corpus as a whole, where that's determinable from
the source material.

SYNTHESIS
=========

PROMPT
CRITIQUE_OUTPUT="${CRITIQUE_DIR}/critique.md"
run_single_stage "critique" "$CRITIQUE_BODY" "$CROSS_OUTPUT" "REFLEXIVE ANALYSIS" "$REFLECTION_OUTPUT" "$CRITIQUE_OUTPUT"

RECON_BODY="${RECONSTRUCTION_DIR}/prompt-body.txt"
cat > "$RECON_BODY" <<'PROMPT'
Reconstruct the Spherepop theory after criticism.

You have three inputs: a corpus-level synthesis; a reflexive
analysis; an adversarial critique. Produce the strongest version of
the theory justified by the available material.

Do not defend every historical claim. Discard weak formulations
where necessary. Separate: axioms/primitives; definitions;
operations; invariants; derived propositions; conjectures;
empirical/computational observations; philosophical interpretations.

Where the critique exposes a genuine gap, preserve the gap
explicitly rather than inventing a solution. Where several terms can
be unified, propose canonical terminology; where terms must remain
distinct, explain why. The objective is conceptual compression
without conceptual loss.

CORPUS SYNTHESIS
================

PROMPT
RECON_OUTPUT="${RECONSTRUCTION_DIR}/reconstruction.md"
# Reconstruction genuinely needs three inputs (synthesis + reflection +
# critique), unlike the other single/double-input stages, so it's built
# directly here rather than through run_single_stage.
if [[ -s "$RECON_OUTPUT" && "$FORCE" != 1 ]]; then
    echo "[cached] $RECON_OUTPUT"
else
    {
        cat "$RECON_BODY"
        cat "$CROSS_OUTPUT"
        echo; echo; echo "REFLEXIVE ANALYSIS"; echo "=================="; echo
        cat "$REFLECTION_OUTPUT"
        echo; echo; echo "ADVERSARIAL CRITIQUE"; echo "===================="; echo
        cat "$CRITIQUE_OUTPUT"
    } > "${RECON_OUTPUT}.prompt.txt"
    pyrun call-model "$DEEP_MODEL" "${RECON_OUTPUT}.prompt.txt" "$RECON_OUTPUT" --cache-dir "$CACHE_DIR" ${FORCE:+--force}
fi

FINAL_BODY="${FINAL_DIR}/prompt-body.txt"
cat > "$FINAL_BODY" <<'PROMPT'
Produce the final research report on the Spherepop repository, useful
to the author as a map of the entire research program. Write
substantial, precise prose.

Include: EXECUTIVE SYNTHESIS; THEORETICAL ARCHITECTURE; FORMAL CORE;
THEORY/IMPLEMENTATION RELATION; INTELLECTUAL DEVELOPMENT;
TERMINOLOGY; STRONGEST RESULTS; WEAKEST LINKS; OPEN PROBLEMS;
CANONICALIZATION (cross-check against manifest/plan.json's canonical
picks rather than re-deriving this from scratch); COMPRESSION (three
increasingly compressed descriptions: ~1000 words, ~250 words, one
paragraph).

Do not introduce unsupported claims merely to make the theory appear
unified.

RECONSTRUCTED THEORY
====================

PROMPT
FINAL_OUTPUT="${FINAL_DIR}/spherepop-theory-report.md"
run_single_stage "final" "$FINAL_BODY" "$RECON_OUTPUT" "ADVERSARIAL CRITIQUE" "$CRITIQUE_OUTPUT" "$FINAL_OUTPUT"

###############################################################################
# Done
###############################################################################

echo
echo "============================================================"
echo " SPHEREPOP REPOSITORY ANALYSIS COMPLETE"
echo "============================================================"
echo
echo "Final report:      $FINAL_OUTPUT"
echo "Canonicalization:   $PLAN_JSON  (check needs_review entries)"
echo "Excluded drafts:    $SUPERSEDED_LIST"
echo "Intermediate stages: $ANALYSIS"
echo
