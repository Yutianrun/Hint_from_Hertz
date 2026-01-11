#!/usr/bin/env bash
set -euo pipefail

# Run-near experiment for NTTRU CPA (ntru_decrypt)
# - Auto-find near z values (low HW after invntt(chat*fhat))
# - Mix with random z values
# - Collect frequency traces via MSR sampling (requires sudo)

TOTAL_LOGICAL_CORES=$(grep '^core id' /proc/cpuinfo | wc -l)

# Parameters matching bak/local_avx/run-local_cpa_nttru.sh
SAMPLES=${SAMPLES:-20000}   # 20000 ms = 20 seconds per selector (CPA default)
OUTER=${OUTER:-10}          # 10 outer loops (CPA default)
THREADS=${THREADS:-8}       # 8 threads (CPA default)
TRIPLE_INDEX=${TRIPLE_INDEX:-0}
TOPK=${TOPK:-3}

DATE_TAG=$(date +"%m%d-%H%M")
TARGET_USER=${SUDO_USER:-$USER}

echo "[*] ===== NTTRU CPA Run-Near Experiment ====="
echo "[*] samples=${SAMPLES} outer=${OUTER} threads=${THREADS} triple_index=${TRIPLE_INDEX} topk=${TOPK}"
echo "[*] Expected runtime ~= $(( (SAMPLES/1000 + 10) * OUTER * 100 / 60 + 10 )) minutes"

make -s

echo "[*] Loading msr module"
sudo modprobe msr

echo "[*] Preparing folders"
sudo rm -rf data/tmp
mkdir -p data/tmp
sudo rm -f input.txt

echo "[*] Finding near z candidates"
SPECIAL_ZS=$(./bin/find_near_z -i "${TRIPLE_INDEX}" -k "${TOPK}" -q)
echo "[*] special_zs: ${SPECIAL_ZS}"

echo "[*] Writing input.txt (100 selectors = special + random)"
python3 - <<PY
import os, random

Q = 7681
special = [int(x) for x in "${SPECIAL_ZS}".split()]
triple_index = int("${TRIPLE_INDEX}")
threads = int("${THREADS}")
total = 100
need_random = total - len(special)
if need_random <= 0:
    raise SystemExit("TOPK too large for total=100")

random.seed(1)
candidates = [z for z in range(Q) if z not in set(special)]
random_z = random.sample(candidates, need_random)

zs = list(special) + random_z
random.shuffle(zs)

with open("input.txt", "w") as f:
    for z in zs:
        f.write(f"{z} {triple_index} {threads}\\n")
PY

echo "[*] Warm-up load (10 minutes)"
if command -v stress-ng >/dev/null 2>&1; then
  stress-ng -q --cpu "${TOTAL_LOGICAL_CORES}" -t 10m
else
  echo "[!] stress-ng not found; skipping warm-up"
fi

echo "[*] Running driver (needs sudo to read MSR)"
sudo ./bin/driver_ntru_avx "${SAMPLES}" "${OUTER}"

echo "[*] Fixing ownership of data/tmp"
sudo chown -R "${TARGET_USER}:${TARGET_USER}" data/tmp

echo "[*] Archiving results"
cp -r data/tmp "data/out-${DATE_TAG}"

echo "[*] Unloading msr module"
sudo modprobe -r msr

echo
echo "[+] Done. Data: data/out-${DATE_TAG}"
echo "[+] Plot: python3 plot.py data/out-${DATE_TAG} --special ${SPECIAL_ZS}"

