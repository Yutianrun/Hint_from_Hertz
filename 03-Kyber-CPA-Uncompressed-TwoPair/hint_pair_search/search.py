#!/usr/bin/env python3
"""
TwoPair Hint Pair Search: Find the hint pair using tournament elimination.
Phase 1: Tournament with fixed pair disabled (z_fixed=0) to find winner pair (a, b)
Phase 2: PK with known z_fixed to determine which of (a, b) is the correct z_target
"""

import argparse
import subprocess
import os
import glob
import numpy as np
from pathlib import Path
import time

KYBER_Q = 3329
KYBER_ROOT_OF_UNITY = 17


def modinv(a, m=KYBER_Q):
    """Modular inverse."""
    return pow(a, -1, m)


def bitreverse7(x: int) -> int:
    """Bit-reversal of a 7-bit integer (0..127)."""
    if x < 0 or x >= 128:
        raise ValueError(f"pair_index out of range: {x} (expected 0..127)")
    out = 0
    for _ in range(7):
        out = (out << 1) | (x & 1)
        x >>= 1
    return out


def expected_pair_product(pair_index: int) -> int:
    """
    For a given pair_index, the two 'half-zero' z values (with (1, z) set at this index)
    satisfy z1 * z2 == 17^(255 - 2*bitreverse7(pair_index)) (mod q).
    """
    exponent = 255 - 2 * bitreverse7(pair_index)
    return pow(KYBER_ROOT_OF_UNITY, exponent, KYBER_Q)


def generate_all_pairs(pair_index: int):
    """Generate all pairs (a, b) where a * b = expected_pair_product(pair_index) (mod q)."""
    target = expected_pair_product(pair_index)
    pairs = []
    for a in range(1, KYBER_Q):
        b = (target * modinv(a, KYBER_Q)) % KYBER_Q
        if b > 0 and a < b:
            pairs.append((a, b))
    return pairs


def parse_freq_file(fn):
    """Parse frequency from output file."""
    freq = []
    with open(fn) as f:
        for line in f:
            parts = line.strip('\x00').split()
            if len(parts) >= 2:
                freq.append(int(parts[1]))
    return np.array(freq)


def measure_pair_tournament(pair, pair_index, samples, outer, thread, parent_dir, data_dir, skip_first=True):
    """Measure a pair during tournament phase (fixed pair disabled; z_fixed=0)."""
    z1, z2 = pair
    tournament_z_fixed = 0

    input_file = os.path.join(parent_dir, "input.txt")
    with open(input_file, 'w') as f:
        f.write(f"{tournament_z_fixed} {z1} {pair_index} {thread}\n")
        f.write(f"{tournament_z_fixed} {z2} {pair_index} {thread}\n")

    tmp_dir = os.path.join(parent_dir, "data", "tmp")
    subprocess.run(f"sudo rm -rf {tmp_dir}", shell=True)
    os.makedirs(tmp_dir, exist_ok=True)

    driver_cmd = f"cd {parent_dir} && sudo ./bin/driver_indcpa_avx {samples} {outer}"
    subprocess.run(driver_cmd, shell=True, capture_output=True)

    subprocess.run(f"sudo chown -R $USER:$USER {tmp_dir}", shell=True)

    all_freq = []
    for z in [z1, z2]:
        pattern = os.path.join(tmp_dir, f"avx_{tournament_z_fixed}_{z}_{pair_index:03d}_*.out")
        files = sorted(glob.glob(pattern))
        if skip_first and len(files) > 1:
            files = files[1:]
        for f in files:
            freq = parse_freq_file(f)
            all_freq.extend(freq)

    pair_dir = os.path.join(data_dir, f"pair_{z1}_{z2}")
    os.makedirs(pair_dir, exist_ok=True)
    subprocess.run(f"mv {tmp_dir}/* {pair_dir}/", shell=True)

    if all_freq:
        return np.mean(all_freq) / 1000000
    return 0


def measure_pk(z_fixed, candidates, pair_index, samples, outer, thread, parent_dir, data_dir, skip_first=True):
    """Measure candidates during PK phase with known z_fixed."""
    input_file = os.path.join(parent_dir, "input.txt")
    with open(input_file, 'w') as f:
        for z in candidates:
            f.write(f"{z_fixed} {z} {pair_index} {thread}\n")

    tmp_dir = os.path.join(parent_dir, "data", "tmp")
    subprocess.run(f"sudo rm -rf {tmp_dir}", shell=True)
    os.makedirs(tmp_dir, exist_ok=True)

    driver_cmd = f"cd {parent_dir} && sudo ./bin/driver_indcpa_avx {samples} {outer}"
    subprocess.run(driver_cmd, shell=True, capture_output=True)

    subprocess.run(f"sudo chown -R $USER:$USER {tmp_dir}", shell=True)

    results = {}
    for z in candidates:
        pattern = os.path.join(tmp_dir, f"avx_{z_fixed}_{z}_{pair_index:03d}_*.out")
        files = sorted(glob.glob(pattern))
        if skip_first and len(files) > 1:
            files = files[1:]
        all_freq = []
        for f in files:
            freq = parse_freq_file(f)
            all_freq.extend(freq)
        if all_freq:
            results[z] = np.mean(all_freq) / 1000000
        else:
            results[z] = 0

    pk_dir = os.path.join(data_dir, "pk")
    os.makedirs(pk_dir, exist_ok=True)
    subprocess.run(f"mv {tmp_dir}/* {pk_dir}/", shell=True)

    return results


def run_tournament(pair_index, samples, outer, thread, parent_dir, output_dir, results_file):
    """Run tournament elimination for a single pair_index."""
    all_pairs = generate_all_pairs(pair_index)
    product = expected_pair_product(pair_index)
    print(f"\n=== Tournament for pair_index={pair_index} (tournament_z_fixed=0) ===")
    print(f"Total pairs: {len(all_pairs)}")
    print(f"Expected product: {product} (mod {KYBER_Q})")

    remaining = all_pairs.copy()
    round_num = 1

    while len(remaining) > 1:
        round_dir = os.path.join(output_dir, f"pair{pair_index}_round_{round_num}")
        os.makedirs(round_dir, exist_ok=True)

        n = len(remaining)
        keep = (n + 1) // 2

        print(f"  Round {round_num}: {n} -> {keep}")
        start_time = time.time()

        scores = []
        for i, pair in enumerate(remaining):
            freq = measure_pair_tournament(pair, pair_index, samples, outer, thread, parent_dir, round_dir)
            scores.append((pair, freq))

            if (i + 1) % 10 == 0 or i == n - 1:
                elapsed = time.time() - start_time
                eta = elapsed / (i + 1) * (n - i - 1)
                print(f"    [{i+1}/{n}] ({pair[0]},{pair[1]}): {freq:.4f} GHz ETA:{eta/60:.1f}m")

        scores.sort(key=lambda x: x[1], reverse=True)
        remaining = [s[0] for s in scores[:keep]]

        elapsed = time.time() - start_time
        print(f"    Done in {elapsed/60:.1f} min, Top: {scores[0]}")

        with open(results_file, 'a') as f:
            f.write(f"pair_index={pair_index} Round {round_num} (tournament_z_fixed=0): {n} -> {keep}\n")
            f.write(f"  Top 3: {scores[:3]}\n")

        round_num += 1

    winner = remaining[0]
    print(f"  Tournament winner: {winner}")
    return winner


def run_pk(winner, z_fixed, pair_index, samples, outer, thread, parent_dir, output_dir, results_file):
    """Run PK phase to determine which value of winner is the correct z_target."""
    candidates = list(winner)
    print(f"\n=== PK for pair_index={pair_index} ===")
    print(f"  Candidates: {candidates}")
    print(f"  z_fixed: {z_fixed}")

    pk_dir = os.path.join(output_dir, f"pair{pair_index}_pk")
    os.makedirs(pk_dir, exist_ok=True)

    # Run PK multiple times for stability
    pk_rounds = 3
    total_scores = {c: 0 for c in candidates}

    for pk_round in range(pk_rounds):
        print(f"  PK round {pk_round + 1}/{pk_rounds}...")
        results = measure_pk(z_fixed, candidates, pair_index, samples, outer, thread, parent_dir, pk_dir)
        for c, freq in results.items():
            total_scores[c] += freq
            print(f"    z={c}: {freq:.4f} GHz")

    # Determine winner
    avg_scores = {c: total_scores[c] / pk_rounds for c in candidates}
    z_target = max(avg_scores, key=avg_scores.get)

    print(f"  PK result: z_target = {z_target}")
    print(f"    {candidates[0]}: {avg_scores[candidates[0]]:.4f} GHz")
    print(f"    {candidates[1]}: {avg_scores[candidates[1]]:.4f} GHz")

    with open(results_file, 'a') as f:
        f.write(f"\npair_index={pair_index} PK Result:\n")
        f.write(f"  z_fixed={z_fixed}\n")
        f.write(f"  {candidates[0]}: {avg_scores[candidates[0]]:.4f} GHz\n")
        f.write(f"  {candidates[1]}: {avg_scores[candidates[1]]:.4f} GHz\n")
        f.write(f"  z_target = {z_target}\n")

    return z_target


def main():
    parser = argparse.ArgumentParser(description='TwoPair hint pair search')
    parser.add_argument('-s', '--samples', type=int, default=100, help='Samples (default: 100)')
    parser.add_argument('-o', '--outer', type=int, default=50, help='Outer (default: 50)')
    parser.add_argument('-z', '--z-fixed', type=int, required=True, help='Known z_fixed for PK phase')
    parser.add_argument('-p', '--pair-indices', type=str, required=True,
                        help='Pair indices to attack (comma-separated, e.g., "5,44,53")')
    args = parser.parse_args()

    pair_indices = [int(p.strip()) for p in args.pair_indices.split(',')]
    z_fixed = args.z_fixed

    script_dir = Path(__file__).parent
    parent_dir = str(script_dir.parent)

    thread = int(subprocess.check_output("grep '^core id' /proc/cpuinfo | wc -l", shell=True).strip())

    run_id = time.strftime("%m%d-%H%M%S")
    output_dir = str(script_dir / "data" / f"search-{run_id}")
    os.makedirs(output_dir, exist_ok=True)

    results_file = os.path.join(output_dir, "results.txt")
    with open(results_file, 'w') as f:
        f.write(f"TwoPair Hint Pair Search\n")
        f.write(f"Parameters: samples={args.samples}, outer={args.outer}\n")
        f.write(f"z_fixed={z_fixed}, pair_indices={pair_indices}\n\n")

    print(f"Output: {output_dir}")
    print(f"z_fixed: {z_fixed}")
    print(f"pair_indices: {pair_indices}")

    subprocess.run("sudo modprobe msr 2>/dev/null || true", shell=True)

    final_results = {}

    try:
        for pair_index in pair_indices:
            # Phase 1: Tournament (fixed pair disabled; z_fixed=0)
            winner = run_tournament(pair_index, args.samples, args.outer, thread,
                                    parent_dir, output_dir, results_file)

            # Phase 2: PK
            z_target = run_pk(winner, z_fixed, pair_index, args.samples, args.outer, thread,
                              parent_dir, output_dir, results_file)

            final_results[pair_index] = z_target

        # Summary
        print("\n=== Final Results ===")
        with open(results_file, 'a') as f:
            f.write(f"\n=== Final Results ===\n")
            for pair_index, z_target in final_results.items():
                print(f"  pair_index={pair_index}: z_target={z_target}")
                f.write(f"  pair_index={pair_index}: z_target={z_target}\n")

    finally:
        subprocess.run(f"sudo rm -f {parent_dir}/input.txt", shell=True)
        subprocess.run(f"sudo rm -rf {parent_dir}/data/tmp", shell=True)
        subprocess.run("sudo modprobe -r msr 2>/dev/null || true", shell=True)


if __name__ == "__main__":
    main()
