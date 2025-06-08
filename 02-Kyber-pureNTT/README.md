# Kyber Pure-NTT Frequency Measurement

* Extract and isolate pure NTT operations from Kyber
* Measure CPU frequency patterns during sustained execution
* Compare results with different polynomial coefficient inputs
## Test Patterns

- Single pair test: r0: [C0, C1, ...], r1: [C2, 0, ...]
- Two pairs test: r0: [C0, C1, C2, C3...], r1: [C4, 0, C5, 0, ...]

## Usage

```bash
# Build
make

# Run measurements
./run_avx.sh        # Single pair AVX2 implementation
./run_ref.sh        # Single pair reference implementation
./run_avx_two_pair.sh  # Two pairs AVX2 implementation
./run_ref_two_pair.sh  # Two pairs reference implementation

# Analyze results
python ./plot.py --steady data/out-avx-${date}
```

Results are saved in the `plot` directory, showing frequency distribution differences based on polynomial coefficient patterns.