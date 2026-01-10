#!/usr/bin/env bash
# TwoPair Hint Pair Search Runner
# Usage: ./run.sh [-z <z_fixed>] [-p <pair_indices>]
# Example: ./run.sh -z 3165 -p "5,44,53"

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

# Default parameters (override via CLI)
SAMPLES=100
OUTER=50

# Default search target (common demo: pair_index=5, z_fixed=3165)
Z_FIXED="3165"
PAIR_INDICES="5"

while [[ $# -gt 0 ]]; do
    case $1 in
        -z|--z-fixed)
            Z_FIXED="$2"
            shift 2
            ;;
        -p|--pair-indices)
            PAIR_INDICES="$2"
            shift 2
            ;;
        -s|--samples)
            SAMPLES="$2"
            shift 2
            ;;
        -o|--outer)
            OUTER="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [-z <z_fixed>] [-p <pair_indices>] [-s samples] [-o outer]"
            echo ""
            echo "Options:"
            echo "  -z, --z-fixed      Known z value from pair0 (default: $Z_FIXED)"
            echo "  -p, --pair-indices Comma-separated pair indices to attack (default: $PAIR_INDICES)"
            echo "  -s, --samples      Samples per measurement (default: $SAMPLES)"
            echo "  -o, --outer        Outer iterations (default: $OUTER)"
            echo ""
            echo "Example:"
            echo "  $0 -z 3165 -p '5,44,53'"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Ensure binary is built
if [[ ! -x "$PARENT_DIR/bin/driver_indcpa_avx" ]]; then
    echo "Building driver..."
    make -C "$PARENT_DIR"
fi

# Warm up CPU
echo "Warming up CPU (5 minutes)..."
TOTAL_LOGICAL_CORES=$(grep '^core id' /proc/cpuinfo | wc -l)
stress-ng -q --cpu "$TOTAL_LOGICAL_CORES" -t 5m

# Run search
echo ""
echo "Starting TwoPair search..."
echo "  z_fixed: $Z_FIXED"
echo "  pair_indices: $PAIR_INDICES"
echo "  samples: $SAMPLES"
echo "  outer: $OUTER"
echo ""

python3 "$SCRIPT_DIR/search.py" \
    -s "$SAMPLES" \
    -o "$OUTER" \
    -z "$Z_FIXED" \
    -p "$PAIR_INDICES"
