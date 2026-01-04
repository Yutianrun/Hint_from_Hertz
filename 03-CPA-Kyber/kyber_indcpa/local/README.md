# Kyber INDCPA Local Experiment

Find the hint pair using CPU frequency side-channel.

## Structure

```
local/
├── hint_pair_search/              # Tournament search for hint pair
│   ├── run.sh                     # Run search (default: s100, o50)
│   └── search.py                  # Search algorithm
├── run_near_s_and_random_100_avx.sh  # Reproduce paper figure
├── driver_indcpa_avx.c            # Measurement driver
├── plot.py                        # Plot results
└── Makefile
```

## Usage

```bash
# Build
make

# Run hint pair search (~12 hours)
cd hint_pair_search && ./run.sh

# Reproduce paper figure: measure 100 z values (98 random + target pair)
sudo ./run_near_s_and_random_100_avx.sh
```

## Scripts

| Script | Function |
|--------|----------|
| `hint_pair_search/run.sh` | Full search, finds hint pair via tournament (~12h) |
| `run_near_s_and_random_100_avx.sh` | Paper figure: 100 z values frequency comparison |

## Parameters (hint_pair_search)

| Param | Default | Description |
|-------|---------|-------------|
| -s | 100 | Samples per measurement |
| -o | 50 | Outer loop iterations |

Tested: s100 o50 achieves 100% win rate for target pair (1760, 2923).
