# Kyber CPA Attack (Uncompressed)

Find the hint pair using CPU frequency side-channel with uncompressed ciphertext.

## Structure

```
03-Kyber-CPA-Uncompressed/
├── patch/                        # Minimal patches for attack
│   ├── craft_ciphertext.c/h      # Ciphertext crafting + uncompressed dec
│   └── test_kyber_indcpa_local.c # Test binary
├── hint_pair_search/             # Tournament search for hint pair
├── driver_indcpa_avx.c           # Measurement driver
├── run_near_s_and_random_100_avx.sh  # Reproduce paper figure
└── Makefile                      # Uses kyber submodule
```

## Usage

```bash
make
cd hint_pair_search && ./run.sh   # Full search (~12h)
sudo ./run_near_s_and_random_100_avx.sh  # Paper figure
```
