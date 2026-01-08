#!/usr/bin/env bash

# NTT frequency measurement
# Usage: ./run.sh [--avx|--ref] [--single|--dual]

TOTAL_LOGICAL_CORES=$(grep '^core id' /proc/cpuinfo | wc -l)

# Default configuration
IMPL="avx"
MODE="single"

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        --avx) IMPL="avx"; shift ;;
        --ref) IMPL="ref"; shift ;;
        --single) MODE="single"; shift ;;
        --dual) MODE="dual"; shift ;;
        -h|--help)
            echo "Usage: $0 [--avx|--ref] [--single|--dual]"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Parameters
samples=8000
outer=10
num_thread=$TOTAL_LOGICAL_CORES
date=$(date +"%m%d-%H%M")

echo "========================================="
echo "NTT Frequency Measurement"
echo "Implementation: $IMPL, Mode: $MODE"
echo "Threads: $num_thread, Samples: $samples, Outer: $outer"
echo "========================================="

# Load MSR
modprobe msr

# Warmup (1 min)
echo "Warming up CPU (1 minute)..."
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 1m

# Prepare output
TMP_DIR="data/tmp"
rm -rf ${TMP_DIR}
mkdir -p ${TMP_DIR}

# Run test
echo "Running tests..."
if [[ "$MODE" == "single" ]]; then
    ./bin/driver_${IMPL} ${num_thread} ${samples} ${outer}
else
    ./bin/driver_${IMPL} --mode dual ${num_thread} ${samples} ${outer}
fi

# Save results
output_dir="data/out-${IMPL}-${MODE}-${date}"
cp -r ${TMP_DIR} ${output_dir}

# Cleanup
modprobe -r msr 2>/dev/null || true

echo "Results saved to: ${output_dir}"
