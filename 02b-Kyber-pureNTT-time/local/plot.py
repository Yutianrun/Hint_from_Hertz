#!/usr/bin/env python3
"""
Simple runtime histogram plotter for 02b experiments.

The expected input is a log file produced by run-local-time_*.sh,
which contains alternating lines such as:
    guess_z 1760
    kyber took about 0.06123 seconds
"""

from __future__ import annotations

import argparse
import os
from collections import defaultdict

import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np


def parse_time_log(path: str) -> dict[int, list[float]]:
    """Parse the timing log and return runtimes (in milliseconds) per guess."""
    data: dict[int, list[float]] = defaultdict(list)
    current_guess: int | None = None

    with open(path, "r", encoding="utf-8", errors="ignore") as fh:
        for raw_line in fh:
            line = raw_line.strip()
            if not line:
                continue

            if "guess_z" in line:
                parts = line.split()
                try:
                    current_guess = int(parts[1])
                except (IndexError, ValueError):
                    current_guess = None

            elif "kyber took about" in line and current_guess is not None:
                parts = line.split()
                try:
                    seconds = float(parts[3])
                except (IndexError, ValueError):
                    continue
                data[current_guess].append(seconds * 1000.0)  # convert to ms

    return data


def plot_histograms(samples: dict[int, list[float]], out_path: str, bins: int, xlim: tuple[float, float] | None):
    """Generate a combined histogram + KDE plot."""
    sns.set_theme(style="whitegrid", font_scale=1.1)
    plt.figure(figsize=(6, 3))

    for guess, values in sorted(samples.items()):
        if not values:
            continue

        arr = np.asarray(values, dtype=float)
        mean = float(np.mean(arr))
        std = float(np.std(arr))
        print(f"guess_z {guess:>5d} ({len(arr):6d} samples): {mean:.6f} ± {std:.6f} ms")

        # Apply a gentle filter (10σ) to remove extreme outliers before plotting.
        filtered = arr[np.abs(arr - mean) <= 10 * std]
        sns.histplot(filtered, bins=bins, stat="density", kde=True, label=str(guess), alpha=0.3)

    plt.xlabel("Runtime (ms)")
    plt.ylabel("Density")
    plt.legend(title="guess_z")
    if xlim:
        plt.xlim(xlim)
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    plt.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot runtime histograms from timing logs.")
    parser.add_argument("logfile", help="Path to the timing log (output of run-local-time_*.sh).")
    parser.add_argument("--bins", type=int, default=100, help="Number of histogram bins (default: 100).")
    parser.add_argument("--output", default="plot/local-time-plot.png", help="Output image path (default: plot/local-time-plot.png).")
    parser.add_argument("--xlim", type=float, nargs=2, metavar=("MIN", "MAX"), help="Optional x-axis limits, in milliseconds.")
    args = parser.parse_args()

    if os.path.isdir(args.logfile):
        raise ValueError(f"Expected a log file, but got directory: {args.logfile}")

    samples = parse_time_log(args.logfile)
    if not samples:
        raise ValueError(f"No timing samples found in {args.logfile}")

    output_dir = os.path.dirname(args.output) or "."
    os.makedirs(output_dir, exist_ok=True)

    plot_histograms(samples, args.output, bins=args.bins, xlim=tuple(args.xlim) if args.xlim else None)


if __name__ == "__main__":
    main()
