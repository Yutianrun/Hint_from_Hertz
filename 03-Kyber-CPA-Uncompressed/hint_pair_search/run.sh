#!/usr/bin/env bash

# Hint Pair Search - Find the hint pair using tournament elimination
# Usage: ./run.sh [-s samples] [-o outer]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default parameters (tested optimal: s100 o50 achieves 100% win rate)
SAMPLES=100
OUTER=100

# Parse arguments
while getopts "s:o:h" opt; do
    case $opt in
        s) SAMPLES=$OPTARG ;;
        o) OUTER=$OPTARG ;;
        h) echo "Usage: $0 [-s samples] [-o outer]"
           echo "  -s  samples (default: 100)"
           echo "  -o  outer (default: 50)"
           exit 0 ;;
        *) exit 1 ;;
    esac
done

# Estimate time: ~13s per pair, 3329 total measurements
TOTAL_PAIRS=3329
TIME_PER_PAIR=13
TOTAL_MIN=$((TOTAL_PAIRS * TIME_PER_PAIR / 60))

echo "=== Hint Pair Search ==="
echo "Parameters: samples=${SAMPLES}, outer=${OUTER}"
echo "Estimated time: ~${TOTAL_MIN} minutes (~$((TOTAL_MIN / 60)) hours)"
echo ""

/home/yu/miniforge3/bin/python3 "${SCRIPT_DIR}/search.py" -s ${SAMPLES} -o ${OUTER}
