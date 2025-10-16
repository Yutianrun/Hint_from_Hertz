# Kyber Pure-NTT Frequency Measurement

* Extract and isolate pure NTT operations from Kyber
* Measure CPU frequency patterns during sustained execution
* Compare results with different polynomial coefficient inputs

## Test Patterns

- Single cipher test: Test one cipher C0
- Dual cipher test: Test a pair of ciphers (C0C0 or C00C configurations)

## Usage

```bash
# Build
make

# Run measurements
./run_avx.sh        # Single cipher AVX2 implementation
./run_ref.sh        # Single cipher reference implementation
./run_avx_dual.sh   # Dual cipher AVX2 implementation
./run_ref_dual.sh   # Dual cipher reference implementation

# Analyze results
python ./plot.py data/out-avx-${date}
```

Results are saved in the `plot` directory, showing frequency distribution differences based on polynomial coefficient patterns.