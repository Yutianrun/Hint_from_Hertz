# Kyber NTT Measurements

Measure NTT operations using two approaches.

## Structure

```
02-Kyber-NTT/
├── freq/   # CPU frequency measurement (DFS leakage)
└── time/   # Execution time measurement
```

## Usage

```bash
# Frequency measurement
cd freq && make && sudo ./run.sh

# Time measurement
cd time && make && ./run_ntt_time.sh
```

See subdirectory READMEs for details.
