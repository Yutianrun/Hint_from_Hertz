# 🔬 Kyber Pure-NTT Frequency Measurement

> Extract and isolate pure NTT operations from Kyber to measure CPU frequency patterns during sustained execution.

## 📋 Overview

This experiment measures CPU frequency variations during Number Theoretic Transform (NTT) operations in Kyber, comparing results with different polynomial coefficient inputs.

**Test Modes:**
- **Single cipher** 🎯: Test one cipher C0 with fixed coefficient positions
- **Dual cipher** 🎲: Test a pair of ciphers (C0C0 or C00C) with random spacing

**Implementations:**
- **AVX2** ⚡: Optimized vectorized implementation
- **Reference** 📚: Portable C implementation

---

## 🚀 Quick Start

### Build

```bash
make
```

### Run Single Test

```bash
# Run with default settings (AVX2 + Single cipher)
sudo ./run.sh

# Or specify implementation and mode
sudo ./run.sh --avx --single    # AVX2 + Single cipher
sudo ./run.sh --ref --dual      # Reference + Dual cipher
```

### Run All Tests (Recommended)

```bash
# Start all 4 tests in background (takes ~1.5-2 hours)
sudo ./start_all.sh

# Monitor progress
tail -f ../log/02-Kyber-pureNTT/run_all.log

# Stop measurements and cleanup
sudo ./start_all.sh stop

# Show help
sudo ./start_all.sh help
```

**All tests includes:**
1. AVX2 + Single Cipher
2. AVX2 + Dual Cipher
3. Reference + Single Cipher
4. Reference + Dual Cipher

Each test takes ~20-25 minutes with 5-minute rest periods between tests.

---

## 📊 Results

### Output Structure

```
data/
├── out-avx-single-MMDD-HHMM/   # AVX2 single cipher results
├── out-avx-dual-MMDD-HHMM/     # AVX2 dual cipher results
├── out-ref-single-MMDD-HHMM/   # Reference single cipher results
└── out-ref-dual-MMDD-HHMM/     # Reference dual cipher results

../log/02-Kyber-pureNTT/run_all.log  # Combined log for all tests
```

### Plot Results

```bash
# Plot specific test results
python plot.py data/out-avx-single-MMDD-HHMM

# Plot with custom output directory
python plot.py data/out-ref-dual-MMDD-HHMM --output custom/path
```

---

## ⚙️ Advanced Usage

### Direct Driver Execution

```bash
# Single cipher mode (default)
sudo ./bin/driver_avx 8 20000 10

# Dual cipher mode
sudo ./bin/driver_avx --mode dual 8 20000 10

# Parameters: <threads> <samples> <iterations>
# - threads: Number of victim threads (default: all logical cores)
# - samples: Number of measurements (20000 = ~20 seconds)
# - iterations: Number of test iterations (10 recommended)
```

### Tuning Parameters

Edit `run.sh` to adjust:
- `samples`: Number of frequency measurements per iteration
- `outer`: Number of test iterations
- `num_thread`: Number of victim threads

---

## 📁 Directory Structure

```
.
├── bin/
│   ├── driver_avx           # AVX2 driver executable
│   ├── driver_ref           # Reference driver executable
│   └── build/               # Intermediate object files
├── data/
│   ├── out-*/               # Test results (timestamped)
│   └── tmp/                 # Temporary output during tests
├── ../kyber/                # Kyber submodule snapshot used for builds
├── driver.c                 # Unified driver implementation
├── run.sh                   # Single test runner
├── run_all.sh               # Run all 4 tests sequentially
├── start_all.sh             # Control script (start/stop/help)
└── plot.py                  # Result visualization
```
---

## 🛠️ Requirements

- **CPU**: Intel with Turbo Boost (tested on i7-9700)
- **OS**: Linux with MSR module support
- **Tools**:
  - `stress-ng` (for CPU warmup)
  - Python 3 with matplotlib, seaborn, numpy
  - AVX2 support (for AVX2 tests)

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt install stress-ng python3-matplotlib python3-seaborn python3-numpy

# Or use conda/mamba
conda install matplotlib seaborn numpy
```

> ℹ️ `../kyber/` is tracked as a Git submodule pointing to [pq-crystals/kyber](https://github.com/pq-crystals/kyber). After updating the submodule always run `make clean && make` so the drivers are rebuilt.

---

## 💡 Notes

- ⚠️ **Requires sudo**: MSR access and CPU frequency control need root privileges
- 🧹 **Cleanup**: Use `sudo ./start_all.sh stop` to stop tests and clean up incomplete data
- ⏱️ **Duration**: Each test takes ~20-25 minutes (10min warmup + measurements)
- 🌡️ **System idle**: Minimize background processes during measurements for best results
- 📊 **Plotting**: After tests complete, manually run `python plot.py data/out-*/` to generate plots

---

## 🐛 Troubleshooting

**Problem**: `modprobe: FATAL: Module msr is builtin`
- ✅ This is normal - MSR module is built into kernel

**Problem**: Tests won't stop with `sudo ./start_all.sh stop`
- Check for orphaned processes: `ps aux | grep -E 'stress-ng|driver_avx|driver_ref'`
- Manual cleanup: `sudo killall -9 stress-ng driver_avx driver_ref`

**Problem**: Permission denied on result files
- Run: `sudo chown -R $USER:$USER data/`
