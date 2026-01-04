# Kyber INDCPA Local Experiment

Find the hint pair using CPU frequency side-channel.

## Structure

```
local/
├── hint_pair_search/              # Tournament search for hint pair
│   ├── run.sh                     # Run search (default: s100, o50)
│   └── search.py                  # Search algorithm
├── run_near_s_and_random_100_avx.sh  # Test near-s and random pairs
├── driver_indcpa_avx.c            # Measurement driver
├── plot.py                        # Plot results
└── Makefile
```

## Usage

```bash
# Build
make

# Run hint pair search (~12 hours)
cd hint_pair_search
./run.sh

# Custom parameters
./run.sh -s 100 -o 50
```

## Parameters

| Param | Default | Description |
|-------|---------|-------------|
| -s | 100 | Samples per measurement |
| -o | 50 | Outer loop iterations |

Tested: s100 o50 achieves 100% win rate for target pair (1760, 2923).
