#!/bin/bash
set -euo pipefail

PROGRESS_FILE="progress.log"
SUMMARY_FILE="source-control.txt"
MAIN_DIR=$(pwd)

log() {
    echo "$*" >> "$MAIN_DIR/$PROGRESS_FILE"
}

is_processed() {
    grep -Fxq "$1" "$MAIN_DIR/$PROGRESS_FILE"
}

process_file() {
    local file=$1
    local dir
    dir=$(dirname "$file")
    local file_name
    file_name=$(basename "$file")

    is_processed "$file_name" && return

    echo "Processing $file_name"
    log "Processing $file_name"

    local sanitized_name
    sanitized_name=$(echo "$file_name" | tr -d '[:space:]')
    local temp_dir
    temp_dir=$(mktemp -d "$dir/tmp_${sanitized_name}_XXXXXX")
    log "Temporary directory created: $temp_dir"

    split -l 1000 "$file" "$temp_dir/chunk_"
    log "File split into chunks: $(find "$temp_dir" -type f | tr '\n' ' ')"

    echo "### ${file_name%.txt}" >> "$MAIN_DIR/$SUMMARY_FILE"
    echo ""                      >> "$MAIN_DIR/$SUMMARY_FILE"

    for chunk_file in "$temp_dir"/chunk_*; do
        [ -f "$chunk_file" ] || continue
        echo "Summarizing chunk: $(basename "$chunk_file")"
        ollama run granite3.2:8b "Summarize in detail and explain:" \
            < "$chunk_file" \
            | tee -a "$MAIN_DIR/$SUMMARY_FILE"
        echo "" >> "$MAIN_DIR/$SUMMARY_FILE"
    done

    rm -rf "$temp_dir"
    log "Temporary directory $temp_dir removed"

    log "$file_name"
}

process_directory() {
    local dir=$1
    echo "Processing directory: $dir"

    for file in "$dir"/*.txt; do
        [ -e "$file" ] || continue
        [ "$(basename "$file")" = "$SUMMARY_FILE" ] && continue
        [ -f "$file" ] || continue
        process_file "$file"
    done

    for subdir in "$dir"/*/; do
        [ -d "$subdir" ] && process_directory "$subdir"
    done
}

# ── main ──────────────────────────────────────────────────────────────────────

touch "$MAIN_DIR/$PROGRESS_FILE"
touch "$MAIN_DIR/$SUMMARY_FILE"

log "Script started at $(date)"
log "Summaries will be saved to $SUMMARY_FILE"

process_directory "$MAIN_DIR"

log "Script completed at $(date)"