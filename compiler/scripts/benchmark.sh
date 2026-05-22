#!/bin/sh
set -e
if [ ! -f ./spherepop ]; then make; fi
for f in benchmarks/*.sp; do
    echo "--- $f ---"
    time ./spherepop "$f"
done
