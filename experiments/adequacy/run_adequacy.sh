#!/usr/bin/env bash
# Phase E adequacy experiment: translate a tiny lambda-calculus fragment
# into Spherepop primitive-event traces, run each trace through the real
# (validated) kernel shared with experiments/flat/, and check that the
# observed result agrees with an independent reference evaluator.
#
# Usage: ./run_adequacy.sh
set -euo pipefail
cd "$(dirname "$0")"
python3 translate_lambda.py --write-traces
