#!/usr/bin/env bash
set -euo pipefail

# Default parameters
WORKLOAD_ITERS=10000
SAMPLES=1000
ROUNDS=5
TRIALS=10
THREADS=8
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/logs"

usage() {
  cat <<'EOF'
Usage: run_ntt_time.sh [options]

Options:
  --workload N       Number of NTT/invNTT iterations per thread (default: 10000)
  --samples N        Number of time measurements per test (default: 1000)
  --rounds N         Number of [C,C] + [C,0] rounds per trial (default: 5)
  --trials N         Number of independent trials (default: 10)
  --threads N        Number of parallel threads per measurement (default: 8)
  -h, --help         Show this help message

Trial structure:
  Each trial performs N rounds
  Each round runs BOTH [C,C] and [C,0] in RANDOM order, collecting M samples each
  Success criterion: For each trial, mean([C,0]) < mean([C,C]) (C,0 is faster)

  Example with default parameters (rounds=5, samples=1000):
    Trial 1:
      Round 1: [C,C] × 1000 samples, [C,0] × 1000 samples (random order)
      Round 2: [C,C] × 1000 samples, [C,0] × 1000 samples (random order)
      ...
      Round 5: [C,C] × 1000 samples, [C,0] × 1000 samples (random order)
      → Total: 5000 [C,C] samples vs 5000 [C,0] samples
      → Compare: mean(all [C,C]) vs mean(all [C,0])

Examples:
  ./run_ntt_time.sh
  ./run_ntt_time.sh --workload 20000 --samples 2000 --rounds 5 --trials 10 --threads 8
EOF
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --workload)
      WORKLOAD_ITERS="$2"
      shift 2
      ;;
    --samples)
      SAMPLES="$2"
      shift 2
      ;;
    --rounds)
      ROUNDS="$2"
      shift 2
      ;;
    --trials)
      TRIALS="$2"
      shift 2
      ;;
    --threads)
      THREADS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
  esac
done

# Create logs directory
mkdir -p "$LOG_DIR"

# Set binary path
BIN_PATH="${SCRIPT_DIR}/test_ntt_time"
MAKE_TARGET="avx2"

# Check binary exists
if [[ ! -x "$BIN_PATH" ]]; then
  echo "Error: executable not found: $BIN_PATH" >&2
  echo "Please compile first: make $MAKE_TARGET" >&2
  exit 1
fi

# Create log file with timestamp
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
LOG_FILE="${LOG_DIR}/ntt_time-avx2-${TIMESTAMP}.log"

# Print configuration
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  NTT Timing Measurement (AVX2)                            ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Configuration:"
echo "  Implementation:      AVX2"
echo "  Workload iterations: ${WORKLOAD_ITERS} (per thread)"
echo "  Samples per test:    ${SAMPLES}"
echo "  Rounds per trial:    ${ROUNDS}"
echo "  Trials:              ${TRIALS}"
echo "  Threads:             ${THREADS}"
echo "  Binary:              ${BIN_PATH}"
echo "  Log file:            ${LOG_FILE}"
echo ""
echo "Trial structure:"
echo "  Each trial has ${ROUNDS} rounds"
echo "  Each round runs BOTH [C,C] and [C,0] in random order"
echo "  Each test collects ${SAMPLES} samples"
echo "  Total per trial:     ${ROUNDS} × 2 patterns × ${SAMPLES} samples = $((ROUNDS * 2 * SAMPLES)) measurements"
echo "  Total overall:       ${TRIALS} trials × $((ROUNDS * 2 * SAMPLES)) = $((TRIALS * ROUNDS * 2 * SAMPLES)) measurements"
echo ""

# Write header to log
{
  echo "# NTT Timing Measurement (AVX2) - $(date)"
  echo "# Configuration:"
  echo "#   implementation = avx2"
  echo "#   workload_iterations = ${WORKLOAD_ITERS}"
  echo "#   samples = ${SAMPLES}"
  echo "#   rounds = ${ROUNDS}"
  echo "#   trials = ${TRIALS}"
  echo "#   threads = ${THREADS}"
  echo "#"
  echo "# Output format: test_num trial round sample time(sec)"
  echo "#   test_num: 0=[C,C], 1=[C,0]"
  echo "#   trial: trial number (1-based)"
  echo "#   round: round number within this trial (1-based)"
  echo "#   sample: sample number within this test (1-based)"
  echo "#   time: measured time in seconds"
  echo "#"
  echo "# Success criterion: mean([C,0]) < mean([C,C])"
  echo "#"
} > "$LOG_FILE"

# Start timing
START_TIME=$(date +%s)

# Run trials
for ((trial=1; trial<=TRIALS; trial++)); do
  echo "════════════════════════════════════════════════════════════"
  echo "Trial ${trial}/${TRIALS}"
  echo "════════════════════════════════════════════════════════════"

  # Run rounds within this trial
  for ((round=1; round<=ROUNDS; round++)); do
    echo ""
    echo "  ──── Round ${round}/${ROUNDS} ────"

    # Randomly decide order: 0 or 1 means [C,C] first or [C,0] first
    if (( RANDOM % 2 )); then
      ORDER=(0 1)
      ORDER_STR="[C,C] then [C,0]"
    else
      ORDER=(1 0)
      ORDER_STR="[C,0] then [C,C]"
    fi

    echo "  Order: ${ORDER_STR}"

    # Run both patterns in the determined order
    for test_num in "${ORDER[@]}"; do
      if [[ $test_num -eq 0 ]]; then
        PATTERN_NAME="[C,C]"
      else
        PATTERN_NAME="[C,0]"
      fi

      echo "    Running test_num=${test_num} (${PATTERN_NAME})..."

      # Run test and capture output
      # stderr goes to terminal (for progress), stdout to variable
      TIMES=$("${BIN_PATH}" "${test_num}" "${WORKLOAD_ITERS}" "${SAMPLES}" "${THREADS}" 2>&1 >/dev/stdout | \
              grep -v '^#' || true)

      # Format and save to log: test_num trial round sample time
      sample=1
      while IFS= read -r time; do
        if [[ -n "$time" ]]; then
          echo "${test_num} ${trial} ${round} ${sample} ${time}" | tee -a "$LOG_FILE"
          ((sample++))
        fi
      done <<< "$TIMES"

      echo "      → Collected $((sample-1)) samples"
    done
  done

  echo ""
done

# End timing
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Completed                                                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo "  Elapsed time:    ${ELAPSED}s"
echo "  Results saved:   ${LOG_FILE}"
echo ""
echo "To analyze results:"
echo "  ./analyze_ntt_time.py ${LOG_FILE}"
echo "  ./analyze_ntt_time.py ${LOG_FILE} --show-trials"
