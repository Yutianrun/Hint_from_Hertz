# Kyber CPA Attack (Uncompressed, Two-Pair)

This directory is intended to be used as the “second stage”: after running
`03-Kyber-CPA-Uncompressed` and obtaining the hint for `pair0`, one value can be
fixed (`z_fixed`) and the target value `z_target` for another `pair_index=k`
can be recovered.

## Approach (two steps per `pair_index`)

1) **Tournament (with `pair0` disabled)**
- `z_fixed` is set to `0` so that `pair0` is not set.
- For the target `pair_index=k`, an elimination tournament is run over candidate
  pairs `(a, b)` until a single winner `winner=(a,b)` remains.
- Each round measures frequency and keeps the top half.

2) **PK (1v1 disambiguation)**
- `pair0=(1, z_fixed)` is fixed (where `z_fixed` comes from the previous stage).
- The two candidates `a` and `b` are measured in a 1v1 run; the higher mean
  frequency is selected as `z_target`.
- Only a single value is reported: `z_target`.

## Defaults / Assumptions

- Default `z_fixed=3165` (for the deterministic key in this repo; the other
  candidate for `pair0` is typically `2246`)
- Default target `pair_index=5`
- Default `samples=100`, `outer=50`

## Running

```bash
make
cd hint_pair_search

./run.sh                 # Defaults: z_fixed=3165, pair_index=5
./run.sh -z 2246 -p 5    # Overrides
```

Results are written to `hint_pair_search/data/search-*/results.txt` (one
`z_target=...` line per `pair_index`).

## Optional: Offline sanity checks

```bash
./bin/check_03_consistency --scan 0                  # Two candidate z values for pair0
./bin/check_03_consistency --scan 5                  # Tournament winner candidates for pair_index=5
./bin/check_03_consistency --scan-two-pair 3165 5    # z_target with pair0 fixed to z_fixed
```
