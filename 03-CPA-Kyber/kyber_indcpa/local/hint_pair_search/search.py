#!/usr/bin/env python3
"""
Hint Pair Search: Find the hint pair using tournament elimination.
Each round measures all pairs, keeps top half by frequency.
"""

import argparse
import subprocess
import os
import glob
import numpy as np
from pathlib import Path
import time


def modinv(a, m=3329):
    """Modular inverse."""
    return pow(a, -1, m)


def generate_all_pairs():
    """Generate all pairs (a, b) where a * b = 17^-1 (mod 3329)."""
    inv17 = modinv(17, 3329)
    pairs = []
    for a in range(1, 3329):
        b = (inv17 * modinv(a, 3329)) % 3329
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


def measure_pair(pair, samples, outer, thread, parent_dir, data_dir, skip_first=True):
    """Measure a pair and return average frequency."""
    z1, z2 = pair

    input_file = os.path.join(parent_dir, "input.txt")
    with open(input_file, 'w') as f:
        f.write(f"1 {z1} 0 {thread}\n")
        f.write(f"1 {z2} 0 {thread}\n")

    tmp_dir = os.path.join(parent_dir, "data", "tmp")
    subprocess.run(f"rm -rf {tmp_dir}", shell=True)
    os.makedirs(tmp_dir, exist_ok=True)

    driver_cmd = f"cd {parent_dir} && sudo ./bin/driver_indcpa_avx {samples} {outer}"
    subprocess.run(driver_cmd, shell=True, capture_output=True)

    subprocess.run(f"sudo chown -R $USER:$USER {tmp_dir}", shell=True)

    all_freq = []
    for z in [z1, z2]:
        pattern = os.path.join(tmp_dir, f"avx_1_{z}_*.out")
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


def run_search(samples, outer, thread, parent_dir, output_dir):
    """Run tournament elimination search."""
    all_pairs = generate_all_pairs()
    print(f"Total pairs: {len(all_pairs)}")

    target = (1760, 2923)
    print(f"Target pair: {target}")
    print(f"Parameters: samples={samples}, outer={outer}")
    print("")

    remaining = all_pairs.copy()
    round_num = 1

    results_file = os.path.join(output_dir, "results.txt")
    with open(results_file, 'w') as f:
        f.write(f"Hint Pair Search Results\n")
        f.write(f"Parameters: samples={samples}, outer={outer}\n")
        f.write(f"Total pairs: {len(all_pairs)}\n\n")

    while len(remaining) > 1:
        round_dir = os.path.join(output_dir, f"round_{round_num}")
        os.makedirs(round_dir, exist_ok=True)

        n = len(remaining)
        keep = (n + 1) // 2

        print(f"=== Round {round_num}: {n} -> {keep} ===")
        start_time = time.time()

        scores = []
        for i, pair in enumerate(remaining):
            freq = measure_pair(pair, samples, outer, thread, parent_dir, round_dir)
            scores.append((pair, freq))

            if (i + 1) % 10 == 0 or i == n - 1:
                elapsed = time.time() - start_time
                eta = elapsed / (i + 1) * (n - i - 1)
                marker = "<<<" if pair == target else ""
                print(f"  [{i+1}/{n}] ({pair[0]},{pair[1]}): {freq:.4f} GHz {marker} ETA:{eta/60:.1f}m")

        scores.sort(key=lambda x: x[1], reverse=True)
        remaining = [s[0] for s in scores[:keep]]

        target_in = target in remaining
        target_rank = next((i+1 for i, s in enumerate(scores) if s[0] == target), -1)

        elapsed = time.time() - start_time
        print(f"  Round {round_num} done in {elapsed/60:.1f} min")
        print(f"  Target rank: {target_rank}/{n} {'(ADVANCED)' if target_in else '(ELIMINATED)'}")
        print(f"  Top 3: {scores[0]}, {scores[1]}, {scores[2] if len(scores) > 2 else 'N/A'}")
        print("")

        with open(results_file, 'a') as f:
            f.write(f"=== Round {round_num}: {n} -> {keep} ===\n")
            f.write(f"Target rank: {target_rank}/{n}\n")
            f.write(f"Top 5:\n")
            for i, (pair, freq) in enumerate(scores[:5]):
                marker = " <-- TARGET" if pair == target else ""
                f.write(f"  {i+1}. ({pair[0]},{pair[1]}): {freq:.4f} GHz{marker}\n")
            f.write(f"\n")

        round_num += 1

    winner = remaining[0]
    print(f"=== WINNER: ({winner[0]}, {winner[1]}) ===")

    with open(results_file, 'a') as f:
        f.write(f"=== WINNER ===\n")
        f.write(f"({winner[0]}, {winner[1]})\n")
        f.write(f"Target (1760, 2923) {'WON!' if winner == target else 'did not win'}\n")

    return winner


def main():
    parser = argparse.ArgumentParser(description='Hint pair search')
    parser.add_argument('-s', '--samples', type=int, default=100, help='Samples (default: 100)')
    parser.add_argument('-o', '--outer', type=int, default=50, help='Outer (default: 50)')
    args = parser.parse_args()

    script_dir = Path(__file__).parent
    parent_dir = str(script_dir.parent)

    thread = int(subprocess.check_output("grep '^core id' /proc/cpuinfo | wc -l", shell=True).strip())

    run_id = time.strftime("%m%d-%H%M%S")
    output_dir = str(script_dir / "data" / f"search-{run_id}")
    os.makedirs(output_dir, exist_ok=True)

    print(f"Output: {output_dir}")
    print("")

    subprocess.run("sudo modprobe msr 2>/dev/null || true", shell=True)

    try:
        winner = run_search(args.samples, args.outer, thread, parent_dir, output_dir)
    finally:
        subprocess.run(f"rm -f {parent_dir}/input.txt", shell=True)
        subprocess.run(f"rm -rf {parent_dir}/data/tmp", shell=True)
        subprocess.run("sudo modprobe -r msr 2>/dev/null || true", shell=True)


if __name__ == "__main__":
    main()
