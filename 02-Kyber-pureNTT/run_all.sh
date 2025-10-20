#!/usr/bin/env bash

# Run all four combinations of NTT frequency measurements
# This script is called by start_all.sh and runs in the background

echo "========================================="
echo "NTT Frequency Measurements - All Tests"
echo "========================================="
echo "Start time: $(date)"
echo ""

start_time=$(date +%s)

# Test 1: AVX2 + Single Cipher
echo "[$(date '+%H:%M:%S')] Test 1/4: AVX2 + Single Cipher"
./run.sh --avx --single
echo "[$(date '+%H:%M:%S')] Test 1/4 completed"
echo ""
sleep 300  # 5 minute rest

# Test 2: AVX2 + Dual Cipher
echo "[$(date '+%H:%M:%S')] Test 2/4: AVX2 + Dual Cipher"
./run.sh --avx --dual
echo "[$(date '+%H:%M:%S')] Test 2/4 completed"
echo ""
sleep 300  # 5 minute rest

# Test 3: Reference + Single Cipher
echo "[$(date '+%H:%M:%S')] Test 3/4: Reference + Single Cipher"
./run.sh --ref --single
echo "[$(date '+%H:%M:%S')] Test 3/4 completed"
echo ""
sleep 300  # 5 minute rest

# Test 4: Reference + Dual Cipher
echo "[$(date '+%H:%M:%S')] Test 4/4: Reference + Dual Cipher"
./run.sh --ref --dual
echo "[$(date '+%H:%M:%S')] Test 4/4 completed"
echo ""

end_time=$(date +%s)
elapsed=$((end_time - start_time))
hours=$((elapsed / 3600))
minutes=$(((elapsed % 3600) / 60))

echo "========================================="
echo "All Tests Completed"
echo "========================================="
echo "End time: $(date)"
echo "Total time: ${hours}h ${minutes}m"
echo ""
echo "Results are saved in data/out-*/"
echo ""
echo "To plot results, run:"
echo "  python plot.py data/out-avx-single-*"
echo "  python plot.py data/out-avx-dual-*"
echo "  python plot.py data/out-ref-single-*"
echo "  python plot.py data/out-ref-dual-*"
echo ""
