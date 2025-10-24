#!/usr/bin/env bash
set -euo pipefail

# Default parameters
THREADS=8
ITERATION=100
OUTER=10
GUESSES=(1760 2011)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/logs"

MODE=""

usage() {
  cat <<'EOF'
Usage: run.sh [options]

Options:
  --avx              Run AVX2 implementation
  --ref              Run reference implementation
  --thread N         Number of worker threads (default: 8)
  --iteration N      Iterations per run (default: 100)
  --outer N          Number of outer repetitions (default: 10)
  --guesses a,b,...  Comma-separated list of guess_z values (default: 1760,2011)
  -h, --help         Show this help message

Examples:
  ./run.sh --avx
  ./run.sh --ref --iteration 200 --outer 10
  ./run.sh --avx --guesses 1760,2011,2923
EOF
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --avx)
      MODE="avx"
      shift
      ;;
    --ref)
      MODE="ref"
      shift
      ;;
    --thread)
      THREADS="$2"
      shift 2
      ;;
    --iteration)
      ITERATION="$2"
      shift 2
      ;;
    --outer)
      OUTER="$2"
      shift 2
      ;;
    --guesses)
      IFS=',' read -ra GUESSES <<< "$2"
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

# Validate mode
if [[ -z "$MODE" ]]; then
  echo "Error: Must specify --avx or --ref" >&2
  usage
  exit 1
fi

if [[ "${#GUESSES[@]}" -eq 0 ]]; then
  echo "Error: No guess_z values provided." >&2
  exit 1
fi

# Create logs directory
mkdir -p "$LOG_DIR"

# Set binary path
if [[ "$MODE" == "avx" ]]; then
  BIN_PATH="${SCRIPT_DIR}/../kyber/avx2/test_kyber_ntt_local"
  IMPL_NAME="AVX2"
else
  BIN_PATH="${SCRIPT_DIR}/../kyber/ref/test_kyber_ntt_local"
  IMPL_NAME="REF"
fi

# Check binary exists
if [[ ! -x "$BIN_PATH" ]]; then
  echo "Error: executable not found: $BIN_PATH" >&2
  echo "Please compile first: cd $(dirname "$BIN_PATH") && make test_kyber_ntt_local" >&2
  exit 1
fi

# Create log file with timestamp
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
LOG_FILE="${LOG_DIR}/time_${MODE}-${TIMESTAMP}.log"

# Print configuration
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Kyber NTT Local Timing Measurement                       ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Configuration:"
echo "  Implementation:  ${IMPL_NAME}"
echo "  Threads:         ${THREADS}"
echo "  Iteration:       ${ITERATION}"
echo "  Outer loops:     ${OUTER}"
echo "  Guess values:    ${GUESSES[*]}"
echo "  Binary:          ${BIN_PATH}"
echo "  Log file:        ${LOG_FILE}"
echo ""

# Estimate runtime
TOTAL_RUNS=$((OUTER * ${#GUESSES[@]}))
echo "Total runs: ${TOTAL_RUNS}"
echo ""

# Start timing
START_TIME=$(date +%s)

# Run experiments
for ((outer_idx=1; outer_idx<=OUTER; outer_idx++)); do
  for guess in "${GUESSES[@]}"; do
    echo "guess_z ${guess}" | tee -a "$LOG_FILE"
    echo "[Outer ${outer_idx}/${OUTER}] Running ${IMPL_NAME} with guess_z=${guess}..."

    # Run and append to log
    "${BIN_PATH}" "${guess}" 0 "${THREADS}" "${ITERATION}" | tee -a "$LOG_FILE"
  done
done

# End timing
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Completed                                                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo "  Elapsed time:    ${ELAPSED}s"
echo "  Results saved:   ${LOG_FILE}"
echo ""
echo "To plot results:"
echo "  ./plot.py ${LOG_FILE}"
