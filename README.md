# Hints by Hertz

DFS leakage toolkit for Hertzbleed-style attacks on Kyber (CPA) and NTTRU (CCA).

## Requirements

- Intel Core i7-9700, Ubuntu 20.04
- Root access for MSR reads
- Python 3 with `matplotlib`, `numpy`, `pandas`
- `stress-ng`, `gcc`, AVX2 support

## Layout

| Directory | Function |
|-----------|----------|
| `01-leakage-channel-data` | Confirm DFS channel on your CPU |
| `02-Kyber-NTT` | NTT frequency & time measurements |
| `03-Kyber-CPA-Uncompressed` | CPA attack on Kyber AVX2 |
| `03-Kyber-KnownSK_*` | Key pair verification |
| `04-NTTRU` | NTTRU CCA attack |
| `05-ToyExample` | Minimal end-to-end demo |
| `06-Bit_Estimation` | DDGR security-loss estimation |

## Quick Start

```bash
# 1. Calibrate leakage
cd 01-leakage-channel-data && sudo ./run-steady.sh

# 2. Measure NTT
cd 02-Kyber-NTT/freq && make && sudo ./run.sh

# 3. Run Kyber CPA attack (~12h)
cd 03-Kyber-CPA-Uncompressed/hint_pair_search && ./run.sh

# 4. Run NTTRU attack
cd 04-NTTRU/local_avx && sudo ./run-local_cca_nttru.sh
```

All scripts require `sudo` for MSR access. Use `--help` for options.
