# NTTRU Frequency Side-Channel Attack

Frequency-based side-channel attack on NTTRU using the "run-near" technique. Adapted from `03-Kyber-CPA-Uncompressed` structure with NTTRU-specific parameters.

## Dependencies

- NTTRU implementation as submodule: `./NTTRU/`
  - From repo root: `git submodule update --init 04-NTTRU/NTTRU`
  - From this dir: `git submodule update --init NTTRU`
  - Optional: `chmod -R a-w NTTRU` to make read-only
- MSR/RAPL sampling utilities: `../util`

## Structure

```
04-NTTRU/
├── patch/
│   ├── test_nttru_local.c        # CPA victim: ntru_decrypt loop
│   ├── test_kem_local.c          # CCA victim: crypto_kem_dec loop
│   ├── find_near_z.c             # White-box tool: find near-z values
│   └── craft_ciphertext*.c/h     # Craft sparse ciphertext
├── driver_ntru_avx.c             # CPA sampling driver
├── driver_kem_avx.c              # CCA sampling driver
├── run_near_f_and_random_100_avx.sh      # CPA experiment
├── run_near_f_and_random_100_avx_cca.sh  # CCA experiment
├── plot.py                       # Visualization
└── Makefile
```

## Quick Start

**CPA Attack:**
```bash
make
./run_near_f_and_random_100_avx.sh
python3 plot.py data/out-MMDD-HHMM
```

**CCA Attack:**
```bash
make
./run_near_f_and_random_100_avx_cca.sh
python3 plot.py data/out-cca-MMDD-HHMM
```

The scripts automatically find near-z values using `bin/find_near_z`, mix them with random z values, and measure CPU frequency during decryption. The plot script defaults to highlighting z values 7359, 942, 6684.

## How It Works

- `find_near_z` is a white-box verification tool that uses the known secret key to enumerate z ∈ [0,Q) and rank by Hamming weight of `crepmod3(invntt(chat*fhat))`
- Crafted ciphertext: `chat = (1, 1, z, 0, ...)`
- Near-z values minimize HW after invntt, causing measurable frequency differences
