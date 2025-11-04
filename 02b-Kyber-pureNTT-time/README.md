# NTT Timing Measurement for Kyber

Simplified NTT timing tests supporting AVX2 implementation without requiring patches.

## Quick Start

```bash
# 1. Build (AVX2 version only)
make

# 2. Run tests
./run_ntt_time.sh                # Use default AVX2 settings

# 3. Analyze results (automatically generates charts)
./analyze_ntt_time.py logs/ntt_time-avx2-*.log
./analyze_ntt_time.py logs/ntt_time-avx2-*.log --show-trials
```

## File Structure

### Core Files
- `test_ntt_time.c` - Unified test program (AVX2 implementation)
- `Makefile` - Build file
- `run_ntt_time.sh` - Test execution script
- `analyze_ntt_time.py` - Result analysis and visualization

### Test Patterns

**Two Coefficient Patterns**:
- Test 0: `[C,C]` - Two random non-zero coefficients
  - AVX2: Positions 0 and 16 (AVX2 paired)
- Test 1: `[C,0]` - Single random non-zero coefficient (position 0)

**Success Criterion**: `mean([C,0]) < mean([C,C])` ([C,0] is faster)

## Test Parameters

### run_ntt_time.sh Options

| Option | Default | Description |
|--------|---------|-------------|
| `--workload N` | `10000` | Number of NTT/invNTT iterations per thread |
| `--samples N` | `10000` | Number of time measurements per test |
| `--rounds N` | `5` | Number of back-and-forth test rounds per trial |
| `--trials N` | `10` | Number of independent trials |
| `--threads N` | `8` | Number of parallel threads |

### Examples

```bash
# Use default parameters (AVX2, 10 trials)
./run_ntt_time.sh

# Custom parameters
./run_ntt_time.sh --workload 20000 --samples 5000 --rounds 5 --trials 10 --threads 8

# Quick test with smaller parameters
./run_ntt_time.sh --workload 100 --samples 100 --rounds 2 --trials 2 --threads 2
```

## Test Structure

### Four-Layer Nesting + Multithreading

```
trials (outermost)      → Success rate statistics (each trial independently judged)
└─ rounds (second layer)   → Back-and-forth testing (multiple rounds per trial)
   └─ samples (third layer) → Time sampling (repeated measurements per round)
      └─ workload (innermost) → Amplify time differences (iterations per thread)
         └─ threads → Parallel threads
```

**Key Points**:
- Each trial contains `rounds` rounds of back-and-forth testing
- Each round: [C,C] and [C,0] each sampled `samples` times (random order)
- Each sample: `threads` threads each run `workload` iterations
- Trial judgment: Combine all rounds data, then compare means

### Data Volume Calculation

#### Default AVX2 Parameters (workload=10000, samples=10000, rounds=5, trials=10, threads=8)

**Per trial**:
- [C,C]: 5 rounds × 10000 samples = **50,000 time samples**
- [C,0]: 5 rounds × 10000 samples = **50,000 time samples**
- Each sample: 8 threads × 10000 iterations = **80,000 NTT/invNTT operations**

**Total**:
- 10 trials × 100,000 samples = **1,000,000 time measurements**
- Total operations: 1,000,000 × 80,000 = **80 billion NTT/invNTT operations**

## Result Analysis

### analyze_ntt_time.py Features

1. **Statistical Analysis**
   - Overall mean, standard deviation, median
   - Percentile comparison (P50, P75, P90, P95, P97.5)
   - t-test and Cohen's d effect size

2. **Success Rate Analysis**
   - Success judgment for each trial
   - Overall success rate
   - Optional: `--show-trials` shows detailed trial information

3. **Visualization** (automatically generated)
   - Histogram + KDE smooth curves
   - Shows 95% data range (P2.5-P97.5)
   - Removes trailing outliers

### Output Files

- **Logs**: `logs/ntt_time-avx2-{timestamp}.log`
  - Format: `test_num trial round sample time(sec)`
- **Charts**: `logs/ntt_time-avx2-{timestamp}.png`
  - Histogram + KDE curves
  - Mean marker lines

## Implementation Details

### AVX2 Characteristics

| Feature | AVX2 |
|---------|------|
| Coefficient Layout | Permuted arrangement |
| [C,C] Positions | 0 and 16 (paired) |
| NTT Implementation | Assembly optimized |
| Performance | High performance |

### Why Different Positions?

- **AVX2**: coeffs[0] and coeffs[16] correspond to the same position in NTT (paired)
- Although positions are different, both test the timing difference between "two non-zero coefficients vs one non-zero coefficient"

## Rounds Feature Explanation

### Why are rounds needed?

**Problem**: Factors like CPU scheduling, cache, and thermal effects cause inaccurate first measurements

**Solution**: Multiple back-and-forth tests of [C,C] and [C,0] within each trial

### How Rounds Work

Each trial executes:
```
Round 1: Random order test [C,C] + [C,0] (samples times each)
Round 2: Random order test [C,C] + [C,0] (samples times each)
...
Round N: Random order test [C,C] + [C,0] (samples times each)

Finally: Combine all rounds data, compare overall means
```

**Advantages**:
- Reduce first-run effects
- Increase data volume, improve statistical significance
- Random order per round, reduce systematic bias

## Cleanup

```bash
make clean        # Clean all build artifacts
make clean-avx2   # Clean AVX2 only
```

## Dependencies

### Build
- GCC with AVX2 support
- pthread

### Analysis Scripts
- Python 3
- numpy
- matplotlib

Install Python dependencies:
```bash
pip install numpy matplotlib
```

## Troubleshooting

### Build Errors
```bash
# Ensure kyber submodule exists
ls ../kyber/avx2/

# Rebuild
make clean && make
```

### Permission Errors
```bash
chmod +x run_ntt_time.sh analyze_ntt_time.py
```

### No Charts Generated
```bash
pip install numpy matplotlib
```

## Project Description

This is part of the Kyber cryptography research project, used to measure timing differences of NTT (Number Theoretic Transform) under different coefficient patterns. Mainly used for side-channel analysis and performance evaluation research.