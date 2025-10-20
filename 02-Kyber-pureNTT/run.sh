#!/usr/bin/env bash

# Unified run script for NTT frequency measurement
# Usage: ./run.sh [--avx|--ref] [--single|--dual]

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Default configuration
IMPL="avx"
MODE="single"

# Parse command line options
while [[ $# -gt 0 ]]; do
    case $1 in
        --avx)
            IMPL="avx"
            shift
            ;;
        --ref)
            IMPL="ref"
            shift
            ;;
        --single)
            MODE="single"
            shift
            ;;
        --dual)
            MODE="dual"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --avx, --ref         Implementation (default: avx)"
            echo "  --single, --dual     Test mode (default: single)"
            echo "  -h, --help           Show this help message"
            echo ""
            echo "Parameters:"
            echo "  samples=20000        Number of measurements (20 seconds)"
            echo "  iterations=10        Number of test iterations"
            echo "  threads=ALL_CORES    Number of victim threads"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Setup
samples=20000		# 20 seconds
outer=10		# 10 iterations
num_thread=$TOTAL_LOGICAL_CORES
date=`date +"%m%d-%H%M"`

echo "========================================="
echo "NTT Frequency Measurement"
echo "========================================="
echo "Implementation: $IMPL"
echo "Mode: $MODE cipher"
echo "Threads: $num_thread"
echo "Samples: $samples"
echo "Iterations: $outer"
echo "========================================="

# Estimate runtime
echo "Estimated time: $(($(($samples/1000+20))*$outer*2/60+10)) minutes"
echo "Reduce 'outer' if you want a shorter run."
echo ""

# Load MSR module
echo "Loading MSR module..."
modprobe msr

# Warmup
echo "Warming up CPU (10 minutes)..."
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Prepare temporary output directory
TMP_DIR="data/tmp"
rm -rf ${TMP_DIR}
mkdir -p ${TMP_DIR}

# Run the test
echo "Running tests..."
if [[ "$MODE" == "single" ]]; then
    ./bin/driver_${IMPL} ${num_thread} ${samples} ${outer}
else
    ./bin/driver_${IMPL} --mode dual ${num_thread} ${samples} ${outer}
fi

# Save results
output_dir="data/out-${IMPL}-${MODE}-${date}"
echo "Saving results to ${output_dir}..."
cp -r ${TMP_DIR} ${output_dir}

# Unload MSR module
echo "Unloading MSR module..."
modprobe -r msr

echo ""
echo "========================================="
echo "Tests completed successfully!"
echo "Results saved to: ${output_dir}"
echo "========================================="
