#!/usr/bin/env python3
"""
Test timing measurement stability across different iteration counts.

Tests whether:
1. Different iteration values produce stable mean separation
2. Order of measurement (1760 first vs 2011 first) affects results
"""

import argparse
import math
import random
import re
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple

# Try to import matplotlib
HAS_MATPLOTLIB = True
try:
    import matplotlib.pyplot as plt
    import numpy as np
except ImportError:
    HAS_MATPLOTLIB = False
    print("Warning: matplotlib not available, plotting disabled", file=sys.stderr)

# Paths
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
LOG_DIR = SCRIPT_DIR / "logs"
PLOT_DIR = SCRIPT_DIR / "plot"

# Regex to extract timing
TIME_PATTERN = re.compile(r"kyber took about ([\d.]+) seconds")


@dataclass
class Logger:
    """Simple logger with timestamp."""
    log_file: Path

    def __post_init__(self):
        self.log_file.parent.mkdir(parents=True, exist_ok=True)
        # Create/truncate log file
        with open(self.log_file, "w") as f:
            f.write(f"Iteration Stability Test - {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write("=" * 80 + "\n\n")

    def log(self, msg: str, to_stdout: bool = True):
        """Log message with timestamp."""
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        line = f"[{timestamp}] {msg}"
        if to_stdout:
            print(line)
        with open(self.log_file, "a", buffering=1) as f:
            f.write(line + "\n")


def run_measurement(
    bin_path: Path,
    guess: int,
    threads: int,
    iteration: int,
) -> List[float]:
    """Run test_kyber_ntt_local and extract timing measurements.

    Args:
        bin_path: Path to binary
        guess: guess_z value (1760 or 2011)
        threads: Number of threads
        iteration: Number of iterations (each outputs one timing line)

    Returns:
        List of timing values in seconds (length = iteration)
    """
    cmd = [
        str(bin_path),
        str(guess),
        "0",  # pair_index
        str(threads),
        str(iteration),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    matches = TIME_PATTERN.findall(result.stdout)
    if not matches:
        raise RuntimeError(f"No timing lines found for: {' '.join(cmd)}")
    return [float(x) for x in matches]


def test_configuration(
    impl: str,
    bin_path: Path,
    iteration: int,
    outer: int,
    threads: int,
    order: str,
    trials: int,
    logger: Logger,
) -> Dict:
    """Test one configuration with specified measurement order.

    Three-level measurement structure:
    1. iteration: How many samples each binary call returns
    2. outer: How many times to call binary to build a distribution (total samples = iteration * outer)
    3. trials: How many times to repeat "build distribution + compare means" to calculate success rate

    Args:
        impl: "avx" or "ref"
        bin_path: Path to binary
        iteration: Number of iterations per run
        outer: Number of outer loops to build one distribution
        threads: Number of threads
        order: "1760_first" or "2011_first"
        trials: Number of trials to repeat the experiment
        logger: Logger instance

    Returns:
        Dict with statistics
    """
    logger.log(
        f"Testing {impl}: iteration={iteration}, outer={outer}, trials={trials}, "
        f"threads={threads}, order={order}",
        to_stdout=True
    )

    # Storage for ALL measurements across all trials (for overall statistics)
    all_samples_1760 = []
    all_samples_2011 = []

    # Track success per trial
    success_count = 0
    trial_results = []

    # Run multiple trials
    for trial_idx in range(trials):
        # Collect samples for this trial (build distribution)
        samples_1760 = []
        samples_2011 = []

        # Measure outer times to build a distribution
        for outer_idx in range(outer):
            # Randomize order for each outer loop
            if order == "random":
                current_order = [1760, 2011] if random.random() < 0.5 else [2011, 1760]
            else:
                current_order = [1760, 2011] if order == "1760_first" else [2011, 1760]

            # Measure in current order
            for guess in current_order:
                times = run_measurement(bin_path, guess, threads, iteration)
                if guess == 1760:
                    samples_1760.extend(times)
                else:
                    samples_2011.extend(times)

        # Now we have a complete distribution (outer * iteration samples each)
        # Calculate means for this trial
        mean_1760 = statistics.mean(samples_1760)
        mean_2011 = statistics.mean(samples_2011)

        # Check if successfully separated (1760 distribution mean < 2011 distribution mean)
        is_success = mean_1760 < mean_2011
        if is_success:
            success_count += 1

        trial_results.append({
            'trial': trial_idx + 1,
            'mean_1760': mean_1760,
            'mean_2011': mean_2011,
            'diff': mean_2011 - mean_1760,
            'success': is_success,
            'num_samples': len(samples_1760)  # Should be outer * iteration
        })

        # Accumulate for overall statistics
        all_samples_1760.extend(samples_1760)
        all_samples_2011.extend(samples_2011)

        logger.log(
            f"  Trial {trial_idx+1}/{trials}: mean_1760={mean_1760:.6f}s, "
            f"mean_2011={mean_2011:.6f}s, diff={mean_2011-mean_1760:.6f}s, "
            f"success={is_success} (samples={len(samples_1760)})",
            to_stdout=False
        )

    # Calculate overall statistics
    success_rate = success_count / trials
    mean_1760_overall = statistics.mean(all_samples_1760)
    mean_2011_overall = statistics.mean(all_samples_2011)
    std_1760 = statistics.stdev(all_samples_1760) if len(all_samples_1760) > 1 else 0.0
    std_2011 = statistics.stdev(all_samples_2011) if len(all_samples_2011) > 1 else 0.0

    # Cohen's d
    pooled_std = math.sqrt((std_1760**2 + std_2011**2) / 2)
    cohens_d = (mean_1760_overall - mean_2011_overall) / pooled_std if pooled_std > 0 else 0.0

    result = {
        'impl': impl,
        'iteration': iteration,
        'outer': outer,
        'trials': trials,
        'threads': threads,
        'order': order,
        'success_rate': success_rate,
        'success_count': success_count,
        'mean_1760': mean_1760_overall,
        'mean_2011': mean_2011_overall,
        'std_1760': std_1760,
        'std_2011': std_2011,
        'diff': mean_2011_overall - mean_1760_overall,
        'cohens_d': cohens_d,
        'samples_1760': all_samples_1760,
        'samples_2011': all_samples_2011,
        'trial_results': trial_results,
    }

    logger.log(
        f"Result: iteration={iteration}, order={order}, success_rate={success_rate:.1%} "
        f"({success_count}/{trials}), diff={result['diff']:.6f}s, cohens_d={cohens_d:.3f}",
        to_stdout=True
    )

    return result


def plot_results(results: List[Dict], output_dir: Path, logger: Logger):
    """Generate plots for iteration stability analysis."""
    if not HAS_MATPLOTLIB:
        logger.log("Matplotlib not available, skipping plots")
        return

    output_dir.mkdir(parents=True, exist_ok=True)

    # Group results by order
    order_1760_first = [r for r in results if r['order'] == '1760_first']
    order_2011_first = [r for r in results if r['order'] == '2011_first']

    # Sort by iteration
    order_1760_first.sort(key=lambda x: x['iteration'])
    order_2011_first.sort(key=lambda x: x['iteration'])

    impl = results[0]['impl']

    # Plot 1: Success Rate vs Iteration
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle(f'Iteration Stability Analysis - {impl.upper()}', fontsize=16)

    # Success Rate
    ax = axes[0, 0]
    iterations_1760 = [r['iteration'] for r in order_1760_first]
    success_1760 = [r['success_rate'] * 100 for r in order_1760_first]
    iterations_2011 = [r['iteration'] for r in order_2011_first]
    success_2011 = [r['success_rate'] * 100 for r in order_2011_first]

    ax.plot(iterations_1760, success_1760, 'o-', label='1760 first', linewidth=2, markersize=8)
    ax.plot(iterations_2011, success_2011, 's-', label='2011 first', linewidth=2, markersize=8)
    ax.axhline(y=80, color='green', linestyle='--', alpha=0.5, label='80% threshold')
    ax.set_xlabel('Iteration Count')
    ax.set_ylabel('Success Rate (%)')
    ax.set_title('Success Rate vs Iteration')
    ax.legend()
    ax.grid(True, alpha=0.3)

    # Cohen's d
    ax = axes[0, 1]
    cohens_d_1760 = [r['cohens_d'] for r in order_1760_first]
    cohens_d_2011 = [r['cohens_d'] for r in order_2011_first]

    ax.plot(iterations_1760, cohens_d_1760, 'o-', label='1760 first', linewidth=2, markersize=8)
    ax.plot(iterations_2011, cohens_d_2011, 's-', label='2011 first', linewidth=2, markersize=8)
    ax.axhline(y=0, color='red', linestyle='-', alpha=0.3)
    ax.axhline(y=-0.8, color='green', linestyle='--', alpha=0.5, label='|d|=0.8 threshold')
    ax.set_xlabel('Iteration Count')
    ax.set_ylabel("Cohen's d")
    ax.set_title("Effect Size vs Iteration")
    ax.legend()
    ax.grid(True, alpha=0.3)

    # Mean Difference
    ax = axes[1, 0]
    diff_1760 = [r['diff'] * 1000 for r in order_1760_first]  # Convert to ms
    diff_2011 = [r['diff'] * 1000 for r in order_2011_first]

    ax.plot(iterations_1760, diff_1760, 'o-', label='1760 first', linewidth=2, markersize=8)
    ax.plot(iterations_2011, diff_2011, 's-', label='2011 first', linewidth=2, markersize=8)
    ax.axhline(y=0, color='red', linestyle='-', alpha=0.3)
    ax.set_xlabel('Iteration Count')
    ax.set_ylabel('Mean Difference (ms)')
    ax.set_title('Mean(2011) - Mean(1760) vs Iteration')
    ax.legend()
    ax.grid(True, alpha=0.3)

    # Standard Deviation
    ax = axes[1, 1]
    std_1760_vals = [r['std_1760'] * 1000 for r in order_1760_first]
    std_2011_vals = [r['std_2011'] * 1000 for r in order_2011_first]

    width = 0.35
    x = np.arange(len(iterations_1760))
    ax.bar(x - width/2, std_1760_vals, width, label='1760 first (1760)', alpha=0.7)
    ax.bar(x + width/2, std_2011_vals, width, label='1760 first (2011)', alpha=0.7)
    ax.set_xlabel('Iteration Count')
    ax.set_ylabel('Standard Deviation (ms)')
    ax.set_title('Measurement Variability')
    ax.set_xticks(x)
    ax.set_xticklabels(iterations_1760)
    ax.legend()
    ax.grid(True, alpha=0.3, axis='y')

    plt.tight_layout()
    output_path = output_dir / f"iteration_stability_{impl}.png"
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

    logger.log(f"Saved plot: {output_path}")

    # Plot 2: Distribution comparison for each iteration
    for result in results:
        if result['order'] != '1760_first':  # Only plot one order to avoid duplication
            continue

        fig, ax = plt.subplots(figsize=(10, 6))
        samples_1760 = np.array(result['samples_1760']) * 1000  # Convert to ms
        samples_2011 = np.array(result['samples_2011']) * 1000

        bins = 50
        ax.hist(samples_1760, bins=bins, alpha=0.5, label='1760', density=True)
        ax.hist(samples_2011, bins=bins, alpha=0.5, label='2011', density=True)

        ax.axvline(result['mean_1760'] * 1000, color='blue', linestyle='--',
                   label=f"Mean 1760: {result['mean_1760']*1000:.3f}ms")
        ax.axvline(result['mean_2011'] * 1000, color='orange', linestyle='--',
                   label=f"Mean 2011: {result['mean_2011']*1000:.3f}ms")

        ax.set_xlabel('Runtime (ms)')
        ax.set_ylabel('Density')
        ax.set_title(f"{impl.upper()}: iteration={result['iteration']}, "
                    f"success_rate={result['success_rate']:.1%}, d={result['cohens_d']:.3f}")
        ax.legend()
        ax.grid(True, alpha=0.3)

        plt.tight_layout()
        output_path = output_dir / f"dist_{impl}_iter{result['iteration']}.png"
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        plt.close()

        logger.log(f"Saved distribution plot: {output_path}")


def print_summary_table(results: List[Dict]):
    """Print summary table of all results."""
    print("\n" + "=" * 120)
    print("SUMMARY TABLE")
    print("=" * 120)

    header = (
        f"{'Impl':<6} {'Iter':>6} {'Order':<12} {'Outer':>5} {'Trials':>6} "
        f"{'SuccRate':>8} {'Mean1760(s)':>12} {'Mean2011(s)':>12} "
        f"{'Diff(ms)':>10} {'Cohen_d':>8} {'Std1760(ms)':>11} {'Std2011(ms)':>11}"
    )
    print(header)
    print("-" * 130)

    for r in results:
        row = (
            f"{r['impl']:<6} {r['iteration']:>6} {r['order']:<12} {r['outer']:>5} {r['trials']:>6} "
            f"{r['success_rate']:>7.1%} {r['mean_1760']:>12.6f} {r['mean_2011']:>12.6f} "
            f"{r['diff']*1000:>10.3f} {r['cohens_d']:>8.3f} "
            f"{r['std_1760']*1000:>11.3f} {r['std_2011']*1000:>11.3f}"
        )
        print(row)

    print("=" * 120)


def main():
    parser = argparse.ArgumentParser(
        description="Test timing measurement stability across different iteration counts"
    )
    parser.add_argument(
        "--iterations",
        type=str,
        default="500,600,700,800,900,1000,1100,1200,1300,1400,1500",
        help="Comma-separated iteration counts to test (default: 500,600,700,800,900,1000,1100,1200,1300,1400,1500)"
    )
    parser.add_argument(
        "--outer",
        type=int,
        default=10,
        help="Number of outer loops to build one distribution (default: 10)"
    )
    parser.add_argument(
        "--trials",
        type=int,
        default=20,
        help="Number of trials to repeat experiment (default: 20)"
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=8,
        help="Number of threads (default: 8)"
    )
    parser.add_argument(
        "--impl",
        choices=["avx", "ref", "both"],
        default="avx",
        help="Implementation to test (default: avx)"
    )
    parser.add_argument(
        "--test-order",
        action="store_true",
        help="Test all measurement orders (1760 first, 2011 first, and random)"
    )
    parser.add_argument(
        "--random-order",
        action="store_true",
        default=True,
        help="Use random order for each outer loop (default: True)"
    )

    args = parser.parse_args()

    # Parse iteration values
    try:
        iterations = [int(x.strip()) for x in args.iterations.split(",")]
    except ValueError:
        print("Error: Invalid iteration values", file=sys.stderr)
        sys.exit(1)

    # Setup logging
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    log_file = LOG_DIR / f"iteration_stability_{timestamp}.log"
    logger = Logger(log_file)

    logger.log(f"Starting iteration stability test")
    logger.log(f"Iterations: {iterations}")
    logger.log(f"Outer: {args.outer} (loops to build one distribution)")
    logger.log(f"Trials: {args.trials} (repeat experiment to calculate success rate)")
    logger.log(f"Threads: {args.threads}")
    logger.log(f"Implementation: {args.impl}")
    logger.log(f"Test order: {args.test_order}")
    logger.log(f"Total samples per distribution: outer × iteration = {args.outer} × iteration")
    logger.log("")

    # Determine which implementations to test
    impls_to_test = []
    if args.impl in ["avx", "both"]:
        impls_to_test.append(("avx", PROJECT_ROOT / "kyber/avx2/test_kyber_ntt_local"))
    if args.impl in ["ref", "both"]:
        impls_to_test.append(("ref", PROJECT_ROOT / "kyber/ref/test_kyber_ntt_local"))

    # Check binaries exist
    for impl, bin_path in impls_to_test:
        if not bin_path.exists():
            logger.log(f"Error: Binary not found: {bin_path}")
            logger.log(f"Please compile: cd {bin_path.parent} && make test_kyber_ntt_local")
            sys.exit(1)

    # Determine measurement orders to test
    if args.test_order:
        orders = ["1760_first", "2011_first", "random"]
    elif args.random_order:
        orders = ["random"]
    else:
        orders = ["1760_first"]

    # Run tests
    all_results = []
    total_configs = len(impls_to_test) * len(iterations) * len(orders)
    config_idx = 0

    for impl, bin_path in impls_to_test:
        impl_results = []

        for iteration in iterations:
            for order in orders:
                config_idx += 1
                logger.log(f"\n{'='*80}")
                logger.log(f"Configuration {config_idx}/{total_configs}")
                logger.log(f"{'='*80}")

                result = test_configuration(
                    impl=impl,
                    bin_path=bin_path,
                    iteration=iteration,
                    outer=args.outer,
                    threads=args.threads,
                    order=order,
                    trials=args.trials,
                    logger=logger,
                )

                impl_results.append(result)
                all_results.append(result)

                # Rest between configs
                if config_idx < total_configs:
                    logger.log(f"Resting 5 seconds before next config...")
                    time.sleep(5)

        # Plot results for this implementation
        logger.log(f"\nGenerating plots for {impl}...")
        plot_results(impl_results, PLOT_DIR, logger)

    # Print summary
    print_summary_table(all_results)

    logger.log(f"\nAll tests completed!")
    logger.log(f"Log file: {log_file}")
    logger.log(f"Plots: {PLOT_DIR}")


if __name__ == "__main__":
    main()
